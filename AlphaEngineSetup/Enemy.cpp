#include "Enemy.h"
#include "AudioSystem.h"
#include <math.h> // For sqrtf
#include <queue>
#include <algorithm>

static AEGfxTexture* s_pEnemyTexture = nullptr;

SimpleEnemy::SimpleEnemy()
    : worldX(0), worldY(0), startX(0), startY(0),
    speed(200.0f), detectionRange(500.0f), giveUpRange(150.0f),
    currentState(EnemyState::IDLE), pMesh(nullptr),
    currentPathIndex(0), pathRecalculateTimer(0.0f),chaseTimer(0.0f),
    hasPatrolTarget(false),maxChaseTime(5.0f),stunTimer(0.0f),
    facingAngle(0.0f), pVisionMesh(nullptr) // FIX: Initialize these here
{
}

SimpleEnemy::~SimpleEnemy()
{
}

void SimpleEnemy::Load()
{
    if (pMesh) return;

    // Load the texture once
    if (!s_pEnemyTexture) {
        s_pEnemyTexture = AEGfxTextureLoad("Assets/enemy.png"); // Adjust path if needed
    }

    AEGfxMeshStart();
    // PURE WHITE MESH
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMesh = AEGfxMeshEnd();

    LoadVisionMesh();
}

void SimpleEnemy::Unload()
{
    // Free the character sprite mesh
    if (pMesh)
    {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }

    if (s_pEnemyTexture) {
        AEGfxTextureUnload(s_pEnemyTexture);
        s_pEnemyTexture = nullptr;
    }

    // Free the vision cone mesh
    if (pVisionMesh) {
        AEGfxMeshFree(pVisionMesh);
        pVisionMesh = nullptr;
    }
}

void SimpleEnemy::SetPosition(float x, float y)
{
    worldX = x;
    worldY = y;
    // Memorize the home location
    startX = x;
    startY = y;
}

void SimpleEnemy::Stun(float duration)
{
    if (duration > stunTimer)
    {
        stunTimer = duration;
    }
}


void SimpleEnemy::Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms)
{
    // stun enemy
    if (stunTimer > 0.0f)
    {
        stunTimer -= dt;
        if (stunTimer < 0.0f)
        {
            stunTimer = 0.0f;
        }
        return;
    }

    // Calculate distance to Player
    float dx = playerX - worldX;
    float dy = playerY - worldY;
    float distToPlayer = sqrtf(dx * dx + dy * dy);

    // Check if player and enemy are in the exact same room
    Room* myRoom = GetRoomFromPos(worldX, worldY, rooms);
    Room* pRoom = GetRoomFromPos(playerX, playerY, rooms);
    bool inSameRoom = (myRoom != nullptr && myRoom == pRoom);
    switch (currentState)
    {
    case EnemyState::IDLE:
        currentState = EnemyState::PATROL;
        break;

    case EnemyState::PATROL:
        // Only notice the player if they are in the exact same room and within range
        if (inSameRoom && distToPlayer < detectionRange) {
            currentState = EnemyState::CHASE;
            chaseTimer = maxChaseTime; // Apply the dice roll duration
            pathRecalculateTimer = 0.0f;
            currentPathIndex = 0;
            currentPath.clear();
        }
        else {
            if (!hasPatrolTarget) {
                if (myRoom) {
                    // Pick a random floor tile in the current room to walk to
                    int rCol = rand() % myRoom->tileCountX;
                    int rRow = rand() % myRoom->tileCountY;

                    if (myRoom->tileMap[rRow][rCol] == 0) {
                        float tx = myRoom->rect.left + (rCol * myRoom->tileSize) + (myRoom->tileSize * 0.5f);
                        float ty = myRoom->rect.top - (rRow * myRoom->tileSize) - (myRoom->tileSize * 0.5f);
                        CalculateAStarPath(tx, ty, rooms);
                        hasPatrolTarget = true;
                    }
                }
            }
            else {
                // Move along the patrol path
                if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
                    AEVec2 targetPoint = currentPath[currentPathIndex];
                    float px = targetPoint.x - worldX;
                    float py = targetPoint.y - worldY;
                    float distToPoint = sqrtf(px * px + py * py);

                    if (distToPoint < 10.0f) {
                        currentPathIndex++;
                    }
                    else {
                        float moveX = (px / distToPoint) * speed * dt;
                        float moveY = (py / distToPoint) * speed * dt;

                        if (IsPositionWalkable(worldX + moveX, worldY, rooms)) worldX += moveX;
                        if (IsPositionWalkable(worldX, worldY + moveY, rooms)) worldY += moveY;
                    }
                }
                else {
                    hasPatrolTarget = false; // Target reached, wait a frame, then pick a new one
                }
            }
        }
        break;

    case EnemyState::CHASE:
        chaseTimer -= dt;

        // Give up if the timer ends
        if (chaseTimer <= 0.0f) {
            currentState = EnemyState::PATROL;
            hasPatrolTarget = false;
            currentPath.clear();
            currentPathIndex = 0;
        }
        else {
            pathRecalculateTimer -= dt;

            if (pathRecalculateTimer <= 0.0f || currentPath.empty()) {
                CalculateAStarPath(playerX, playerY, rooms);
                pathRecalculateTimer = 0.15f; // 0.1s is fine, 0.15s is often smoother
            }

            if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
                AEVec2 targetPoint = currentPath[currentPathIndex];
                float px = targetPoint.x - worldX;
                float py = targetPoint.y - worldY;
                float distToPoint = sqrtf(px * px + py * py);

                // Increase this to 15.0f or 20.0f if the enemy "orbits" the waypoint
                if (distToPoint < 15.0f) {
                    currentPathIndex++;
                }
                else {
                    float dirX = px / distToPoint;
                    float dirY = py / distToPoint;
                    float moveX = dirX * speed * dt;
                    float moveY = dirY * speed * dt;

                    if (IsPositionWalkable(worldX + moveX, worldY, rooms)) worldX += moveX;
                    if (IsPositionWalkable(worldX, worldY + moveY, rooms)) worldY += moveY;
                }
            }
            else {
                // FALLBACK: If A* fails (e.g. player stands on a wall edge), 
                // nudge the enemy directly toward the player so they don't freeze.
                float dxFallback = playerX - worldX;
                float dyFallback = playerY - worldY;
                float d = sqrtf(dxFallback * dxFallback + dyFallback * dyFallback);
                if (d > 1.0f) {
                    // --- SLIDING COLLISION: FALLBACK ---
                    float moveX = (dxFallback / d) * speed * dt;
                    float moveY = (dyFallback / d) * speed * dt;

                    if (IsPositionWalkable(worldX + moveX, worldY, rooms)) worldX += moveX;
                    if (IsPositionWalkable(worldX, worldY + moveY, rooms)) worldY += moveY;
                }
            }
        }
        break;

    case EnemyState::RETURN:
        currentState = EnemyState::PATROL; // Safely fall back to patrolling
        break;
    }
}

void SimpleEnemy::Draw()
{
    if (!pMesh) return;

    // --- SETUP TEXTURE RENDERING ---
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetTransparency(1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxTextureSet(s_pEnemyTexture, 0, 0);

    // Keep your state colors to tint the enemy sprite!
    if (IsStunned())
        AEGfxSetColorToMultiply(0.0f, 0.5f, 1.0f, 1.0f);
    else if (currentState == EnemyState::IDLE)
        AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
    else if (currentState == EnemyState::PATROL)
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f); // Pure white while patrolling
    else if (currentState == EnemyState::CHASE)
        AEGfxSetColorToMultiply(1.0f, 0.2f, 0.2f, 1.0f); // Angry red tint!
    else if (currentState == EnemyState::RETURN)
        AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f);

    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 70.0f, 70.0f); // Made slightly bigger for the sprite
    AEMtx33Trans(&trans, worldX, worldY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);

    // --- RESET ---
    AEGfxTextureSet(nullptr, 0, 0);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
}

bool SimpleEnemy::IsPosValid(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
    // A simple collision radius so the enemy doesn't clip halfway into walls
    float padding = 10.0f;

    for (const auto& room : rooms)
    {
        // Is the point inside this room's boundaries?
        if (x >= room->rect.left && x <= room->rect.right &&
            y >= room->rect.bottom && y <= room->rect.top)
        {
            // Convert World (x, y) to Grid (col, row)
            int col = static_cast<int>((x - room->rect.left) / room->tileSize);
            int row = static_cast<int>((room->rect.top - y) / room->tileSize);

            // Safety bounds check
            if (col >= 0 && col < room->tileCountX && row >= 0 && row < room->tileCountY)
            {
                // Return true ONLY if the tile is a floor (0)
                return room->tileMap[row][col] == 0;
            }
        }
    }
    // If it's not inside any room, it's out of bounds (invalid)
    return false;
}

Room* SimpleEnemy::GetRoomFromPos(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
    for (const auto& room : rooms) {
        if (x >= room->rect.left && x <= room->rect.right &&
            y >= room->rect.bottom && y <= room->rect.top) {
            return room.get();
        }
    }
    return nullptr;
}

void SimpleEnemy::CalculateAStarPath(float targetX, float targetY, const std::vector<std::unique_ptr<Room>>& rooms)
{
    currentPath.clear();
    currentPathIndex = 0;

    Room* startRoom = GetRoomFromPos(worldX, worldY, rooms);
    Room* targetRoom = GetRoomFromPos(targetX, targetY, rooms);


    // Convert World space to Grid space
    int startCol = static_cast<int>((worldX - startRoom->rect.left) / startRoom->tileSize);
    int startRow = static_cast<int>((startRoom->rect.top - worldY) / startRoom->tileSize);
    int targetCol = static_cast<int>((targetX - startRoom->rect.left) / startRoom->tileSize);
    int targetRow = static_cast<int>((startRoom->rect.top - targetY) / startRoom->tileSize);

    int cols = startRoom->tileCountX;
    int rows = startRoom->tileCountY;

    // Bounds check
    if (startCol < 0 || startCol >= cols || startRow < 0 || startRow >= rows ||
        targetCol < 0 || targetCol >= cols || targetRow < 0 || targetRow >= rows) return;

    struct Node {
        int r, c;
        float g, f;
        int parentR, parentC;
        bool closed, open;
    };

    std::vector<Node> grid(cols * rows);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int idx = r * cols + c;
            grid[idx] = { r, c, 99999.0f, 99999.0f, -1, -1, false, false };
        }
    }

    auto comp = [&grid, cols](std::pair<int, int> a, std::pair<int, int> b) {
        return grid[a.first * cols + a.second].f > grid[b.first * cols + b.second].f;
        };
    std::priority_queue<std::pair<int, int>, std::vector<std::pair<int, int>>, decltype(comp)> openList(comp);

    int startIdx = startRow * cols + startCol;
    grid[startIdx].g = 0;
    grid[startIdx].f = static_cast<float>(std::abs(targetRow - startRow) + std::abs(targetCol - startCol));
    grid[startIdx].open = true;
    openList.push({ startRow, startCol });

    bool found = false;
    int dr[] = { -1, 1, 0, 0 }; // Up, Down
    int dc[] = { 0, 0, -1, 1 }; // Left, Right

    while (!openList.empty()) {
        auto curr = openList.top();
        openList.pop();

        int r = curr.first;
        int c = curr.second;
        int idx = r * cols + c;

        if (r == targetRow && c == targetCol) {
            found = true;
            break;
        }

        grid[idx].closed = true;

        for (int i = 0; i < 4; ++i) {
            int nr = r + dr[i];
            int nc = c + dc[i];

            if (nr < 0 || nr >= rows || nc < 0 || nc >= cols) continue;
            if (startRoom->tileMap[nr][nc] == 1) continue; // 1 is a wall

            int nIdx = nr * cols + nc;
            if (grid[nIdx].closed) continue;

            float tentative_g = grid[idx].g + 1.0f;

            if (tentative_g < grid[nIdx].g) {
                grid[nIdx].parentR = r;
                grid[nIdx].parentC = c;
                grid[nIdx].g = tentative_g;
                grid[nIdx].f = tentative_g + std::abs(targetRow - nr) + std::abs(targetCol - nc);

                if (!grid[nIdx].open) {
                    grid[nIdx].open = true;
                    openList.push({ nr, nc });
                }
            }
        }
    }

    // Reconstruct path
    if (found) {
        int currR = targetRow;
        int currC = targetCol;

        while (currR != startRow || currC != startCol) {
            // Convert grid space back to world space for movement target
            float wx = startRoom->rect.left + (currC * startRoom->tileSize) + (startRoom->tileSize * 0.5f);
            float wy = startRoom->rect.top - (currR * startRoom->tileSize) - (startRoom->tileSize * 0.5f);

            currentPath.push_back({ wx, wy });

            int pR = grid[currR * cols + currC].parentR;
            int pC = grid[currR * cols + currC].parentC;
            currR = pR;
            currC = pC;
        }
        // A* builds the path backwards, so we reverse it
        std::reverse(currentPath.begin(), currentPath.end());
    }
}

bool SimpleEnemy::IsPositionWalkable(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
    for (const auto& room : rooms)
    {
        // Check if inside Room Bounding Box
        if (x >= (float)room->rect.left && x <= (float)room->rect.right &&
            y >= (float)room->rect.bottom && y <= (float)room->rect.top)
        {
            // Calculate which tile grid index the entity is on
            // Local Position equals World Pos minus Room Top Left
            float localX = x - (float)room->rect.left;
            float localY = (float)room->rect.top - y;

            int tileX = static_cast<int>(localX / room->tileSize);
            int tileY = static_cast<int>(localY / room->tileSize);

            // Check the specific tile value where 0 is Floor and 1 is Wall
            if (room->GetTile(tileX, tileY) == 0)
            {
                return true;
            }
            else
            {
                return false;
            }
        }
    }
    // Void outside all rooms is treated as a wall
    return false;
}
void SimpleEnemy::LoadVisionMesh() {
    if (pVisionMesh) return;

    int segments = 24; // Increased slightly for a smoother wide curve

    // WIDER: Changed from 45 degrees to 90 degrees
    float coneAngle = 90.0f * (3.14159265f / 180.0f);

    // SHORTER: Hardcoded to 300 instead of the massive 500 detectionRange
    float visionDist = 300.0f;

    AEGfxMeshStart();
    // The Tip of the cone (at enemy center)
    for (int i = 0; i < segments; ++i) {
        float a1 = -coneAngle / 2.0f + (i * coneAngle / segments);
        float a2 = -coneAngle / 2.0f + ((i + 1) * coneAngle / segments);

        // Triangle: Center -> Point1 on arc -> Point2 on arc
        AEGfxTriAdd(0.0f, 0.0f, 0x88FF0000, 0, 0,
            cosf(a1) * visionDist, sinf(a1) * visionDist, 0x00FF0000, 0, 0,
            cosf(a2) * visionDist, sinf(a2) * visionDist, 0x00FF0000, 0, 0);
    }
    pVisionMesh = AEGfxMeshEnd();
}

void SimpleEnemy::DrawVisionOverlay() {
    // Only draw if the mesh exists and the enemy is chasing
    if (!pVisionMesh || currentState != EnemyState::CHASE || stunTimer > 0.0f) return;

    // Update facing angle based on movement direction
    if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
        AEVec2 target = currentPath[currentPathIndex];
        facingAngle = atan2f(target.y - worldY, target.x - worldX);
    }

    // Build the transformation matrix
    AEMtx33 scale, rot, trans, transform;
    AEMtx33Scale(&scale, 1.0f, 1.0f);
    AEMtx33Rot(&rot, facingAngle);
    AEMtx33Trans(&trans, worldX, worldY);
    
    // Combine translation and rotation
    AEMtx33Concat(&transform, &rot, &scale); // Scale then rotate
    AEMtx33Concat(&transform, &trans, &transform); // Then translate

    // Set render states
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_ADD);
    
    // Apply transform and draw the existing mesh
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pVisionMesh, AE_GFX_MDM_TRIANGLES);
    
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
}