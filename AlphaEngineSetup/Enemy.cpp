#include "Enemy.h"
#include <math.h> // For sqrtf
#include <queue>
#include <algorithm>

SimpleEnemy::SimpleEnemy()
    : worldX(0), worldY(0), startX(0), startY(0),
    speed(200.0f), detectionRange(500.0f), giveUpRange(150.0f),
    currentState(EnemyState::IDLE), pMesh(nullptr),
    currentPathIndex(0), pathRecalculateTimer(0.0f) // Initialize pathing vars
{
}

SimpleEnemy::~SimpleEnemy()
{
    Unload();
}

void SimpleEnemy::Load()
{
    if (pMesh) return;
    AEGfxMeshStart();
    // Red Square Mesh
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF0000FF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFF0000FF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFF0000FF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFF0000FF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFF0000FF, 0.0f, 0.0f);
    pMesh = AEGfxMeshEnd();
}

void SimpleEnemy::Unload()
{
    pMesh = nullptr;
}

void SimpleEnemy::SetPosition(float x, float y)
{
    worldX = x;
    worldY = y;
    // Memorize the home location
    startX = x;
    startY = y;
}

void SimpleEnemy::Update(float playerX, float playerY, float dt, const std::vector<std::unique_ptr<Room>>& rooms)
{
    float dx = playerX - worldX;
    float dy = playerY - worldY;
    float distToPlayer = sqrtf(dx * dx + dy * dy);

    float hx = startX - worldX;
    float hy = startY - worldY;
    float distToHome = sqrtf(hx * hx + hy * hy);

    switch (currentState)
    {
    case EnemyState::IDLE:
        if (distToPlayer < detectionRange) {
            currentState = EnemyState::CHASE;
            pathRecalculateTimer = 0.0f; // Force immediate path calculation
        }
        break;

    case EnemyState::CHASE:
        if (distToPlayer > giveUpRange) {
            currentState = EnemyState::RETURN;
            currentPath.clear(); // Clear path when giving up
        }
        else {
            // Recalculate A* path every 0.5 seconds to track moving player
            pathRecalculateTimer -= dt;
            if (pathRecalculateTimer <= 0.0f) {
                CalculateAStarPath(playerX, playerY, rooms);
                pathRecalculateTimer = 0.5f;
            }

            // Move along the calculated A* path
            if (!currentPath.empty() && currentPathIndex < currentPath.size()) {
                AEVec2 targetPoint = currentPath[currentPathIndex];
                float px = targetPoint.x - worldX;
                float py = targetPoint.y - worldY;
                float distToPoint = sqrtf(px * px + py * py);

                if (distToPoint < 10.0f) { // Reached current waypoint, go to next
                    currentPathIndex++;
                }
                else {
                    float dirX = px / distToPoint;
                    float dirY = py / distToPoint;

                    worldX += dirX * speed * dt;
                    worldY += dirY * speed * dt;
                }
            }
            else {
                // Fallback: If no path found or very close, use direct chase
                if (distToPlayer > 0.01f) {
                    float dirX = dx / distToPlayer;
                    float dirY = dy / distToPlayer;
                    float nextX = worldX + (dirX * speed * dt);
                    float nextY = worldY + (dirY * speed * dt);

                    if (IsPosValid(nextX, worldY, rooms)) worldX = nextX;
                    if (IsPosValid(worldX, nextY, rooms)) worldY = nextY;
                }
            }
        }
        break;

    case EnemyState::RETURN:
        // (Keep your existing RETURN state logic here)
        if (distToPlayer < detectionRange) {
            currentState = EnemyState::CHASE;
            pathRecalculateTimer = 0.0f;
        }
        else if (distToHome < 10.0f) {
            worldX = startX;
            worldY = startY;
            currentState = EnemyState::IDLE;
        }
        else {
            float dirX = hx / distToHome;
            float dirY = hy / distToHome;
            float nextX = worldX + (dirX * speed * dt);
            float nextY = worldY + (dirY * speed * dt);

            if (IsPosValid(nextX, worldY, rooms)) worldX = nextX;
            if (IsPosValid(worldX, nextY, rooms)) worldY = nextY;
        }
        break;
    }
}

void SimpleEnemy::Draw()
{
    if (!pMesh) return;

    // Change color based on state for visual feedback
    if (currentState == EnemyState::IDLE)
        AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f); // Grey (Sleeping)
    else if (currentState == EnemyState::CHASE)
        AEGfxSetColorToMultiply(1.0f, 0.2f, 0.2f, 1.0f); // Bright Red (Angry)
    else if (currentState == EnemyState::RETURN)
        AEGfxSetColorToMultiply(1.0f, 1.0f, 0.0f, 1.0f); // Yellow (Confused/Returning)

    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 25.0f, 25.0f);
    AEMtx33Trans(&trans, worldX, worldY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
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

    // Only run A* if both entities are inside the exact same room
    if (!startRoom || startRoom != targetRoom) return;

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