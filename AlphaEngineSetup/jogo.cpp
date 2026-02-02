#include "jogo.h"
#include "items.h"
#include <iostream>
#include <cmath> // Required for math functions like fabsf and sinf

Character::Character(int startX, int startY, float tile)
    : gridX(startX), gridY(startY), tileSize(tile), pMesh(nullptr), moveTimer(0.0f)
{
    // Initialize world position to match the starting grid center
    // This places the "float" position exactly in the middle of the tile
    worldX = (static_cast<float>(gridX) * tileSize) + (tileSize * 0.5f);
    worldY = (static_cast<float>(gridY) * tileSize) + (tileSize * 0.5f);
    moveSpeed = 400.0f; // Walking speed in world units
    isMoving = false;
}

Character::~Character()
{
    // Destructor ensures GPU resources are released if the object is destroyed
    Unload();
}

void Character::Load()
{
    if (pMesh != nullptr) return;

    // Build the player mesh: a simple centered square
    // The mesh is defined in local space (-0.5 to 0.5) to make scaling and rotation easier
    AEGfxMeshStart();

    // Vertices are assigned a Cyan color (0xFF00FFFF) for high visibility
    AEGfxTriAdd(-0.5f, -0.5f, 0xFF00FFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFF00FFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFF00FFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFF00FFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFF00FFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFF00FFFF, 0.0f, 0.0f);

    pMesh = AEGfxMeshEnd();
}

void Character::Unload()
{
    // Release the character mesh from the GPU memory to prevent memory leaks
    if (pMesh)
    {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}

void Character::Update(const std::vector<std::unique_ptr<Room>>& rooms)
{
    // Retrieve delta time for frame-rate independent movement
    float dt = static_cast<float>(AEFrameRateControllerGetFrameTime());

    float dirX = 0.0f;
    float dirY = 0.0f;

    // Direct input detection: character moves as long as keys are held
    if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP))   dirY += 1.0f;
    if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN)) dirY -= 1.0f;
    if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT)) dirX -= 1.0f;
    if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) dirX += 1.0f;

    // Only process movement if there is active input
    if (dirX != 0.0f || dirY != 0.0f)
    {
        isMoving = true;

        // Normalize the direction vector to prevent faster diagonal movement
        float length = sqrtf(dirX * dirX + dirY * dirY);
        dirX /= length;
        dirY /= length;

        // Calculate potential new position
        float nextWorldX = worldX + (dirX * moveSpeed * dt);
        float nextWorldY = worldY + (dirY * moveSpeed * dt);

        // COLLISION CHECK: Look ahead to see if the next position is inside a valid room/corridor
        if (IsPointInsideAnyRoom(nextWorldX, nextWorldY, rooms))
        {
            worldX = nextWorldX;
            worldY = nextWorldY;

            // Sync grid coordinates (useful for mini-maps or room logic)
            gridX = static_cast<int>(floorf(worldX / tileSize));
            gridY = static_cast<int>(floorf(worldY / tileSize));

            // Update animation timer for the walking gait (bob/tilt)
            moveTimer += dt * 12.0f;

            // Fog of War: Reveal the room the player is currently physically inside
            for (const auto& room : rooms)
            {
                if (worldX >= room->rect.left && worldX <= room->rect.right &&
                    worldY >= room->rect.bottom && worldY <= room->rect.top)
                {
                    room->isDiscovered = true;
                }
            }
        }
    }
    else
    {
        // Reset animation state when movement stops
        isMoving = false;
        moveTimer = 0.0f;
    }
}

void Character::Draw()
{
    if (!pMesh) return;

    AEMtx33 scale, rot, trans, transform;

    // Calculate secondary animation effects (bobbing and tilting)
    float bobOffset = isMoving ? fabsf(sinf(moveTimer)) * 8.0f : 0.0f;
    float tiltAngle = isMoving ? sinf(moveTimer) * 0.1f : 0.0f;

    // Apply the animated scale, rotation, and translation
    AEMtx33Scale(&scale, tileSize * 0.4f, tileSize * 0.4f);
    AEMtx33Rot(&rot, tiltAngle);
    AEMtx33Trans(&trans, worldX, worldY + bobOffset);

    // Matrix Concatenation: Scale -> Rotate -> Translate
    AEMtx33Concat(&transform, &rot, &scale);
    AEMtx33Concat(&transform, &trans, &transform);

    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetTransform(transform.m);

    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
}

void Character::SetPosition(int x, int y)
{
    // Snap both the grid and world coordinates (teleport)
    gridX = x;
    gridY = y;
    worldX = (static_cast<float>(gridX) * tileSize) + (tileSize * 0.5f);
    worldY = (static_cast<float>(gridY) * tileSize) + (tileSize * 0.5f);
}

void Character::CollectItem(ItemsManager& itemsManager)
{
    // Use the real-time world position for high-precision item collection
    if (AEInputCheckTriggered(AEVK_E))
    {
        itemsManager.Update(worldX, worldY, 0.0f);
    }
}

bool Character::IsPointInsideAnyRoom(float x, float y, const std::vector<std::unique_ptr<Room>>& rooms)
{
    // Iterates through all rooms to see if the point (x,y) is walkable
    for (const auto& room : rooms)
    {
        if (x >= (float)room->rect.left && x <= (float)room->rect.right &&
            y >= (float)room->rect.bottom && y <= (float)room->rect.top)
        {
            return true;
        }
    }
    return false;
}

//Character::Character(int startX, int startY, float tile)
//    : gridX(startX), gridY(startY), tileSize(tile), pMesh(nullptr),
//    moveTimer(0.0f), moveDelay(0.10f) {
//}
//
//Character::~Character() {
//    Unload();
//}
//
//void Character::Load() {
//
//    if (pMesh != nullptr) return;
//
//    // Create a white square mesh
//    AEGfxMeshStart();
//
//    AEGfxTriAdd(
//        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
//        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//    );
//
//    AEGfxTriAdd(
//        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
//        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//    );
//
//    pMesh = AEGfxMeshEnd();
//}
//
//void Character::Unload() {
//    if (pMesh) {
//        AEGfxMeshFree(pMesh);
//        pMesh = nullptr;
//    }
//}
//
//bool Character::IsValidPosition(int x, int y, const std::array<std::array<int, 20>, 15>& maze) {
//    // Check bounds
//    if (x < 0 || x >= 20 || y < 0 || y >= 15) {
//        return false;
//    }
//
//    // Check if it's not a wall (0 = no wall, 1 = wall)
//    return maze[y][x] == 0;
//}
//
//void Character::Update(const std::array<std::array<int, 20>, 15>& maze) {
//    // Update timer
//    moveTimer += (f32)AEFrameRateControllerGetFrameTime();
//
//    // Only allow movement if enough time has passed
//    if (moveTimer >= moveDelay) {
//        int newX = gridX;
//        int newY = gridY;
//        bool moved = false;
//
//        // Check for key presses and calculate new position
//        if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP)) {
//            newY += 1;
//            moved = true;
//        }
//        else if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN)) {
//            newY -= 1;
//            moved = true;
//        }
//
//        if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT)) {
//            newX -= 1;
//            moved = true;
//        }
//        else if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) {
//            newX += 1;
//            moved = true;
//        }
//
//        // Only update position if the new position is valid (not a wall)
//        if (moved && IsValidPosition(newX, newY, maze)) {
//            gridX = newX;
//            gridY = newY;
//            moveTimer = 0.0f;
//        }
//    }
//}
//
//
//void Character::CollectItem(ItemsManager& itemsManager) {
//    // Check if E key is pressed (triggered once per press)
//    if (AEInputCheckTriggered(AEVK_E)) {
//
//        float playerX = GetWorldX();
//        float playerY = GetWorldY();
//        itemsManager.Update(playerX, playerY, 0.0f);
//    }
//}
//
//
//void Character::Draw() {
//
//    // If pMesh is null, calling AEGfxMeshDraw will make the game exit/crash.
//    if (!pMesh) return;
//
//    // Calculate world position from grid position
//    float drawX = (gridX * tileSize) - (f32)AEGfxGetWindowWidth() / 2.0f + (tileSize / 2.0f);
//    float drawY = (gridY * tileSize) - (f32)AEGfxGetWindowHeight() / 2.0f + (tileSize / 2.0f);
//
//    // Create transformation matrices
//    AEMtx33 scale, trans, transform;
//
//    // Scale to tile size
//    AEMtx33Scale(&scale, (f32)(tileSize) * 0.8f, (f32)(tileSize) * 0.8f);
//    AEMtx33Trans(&trans, drawX, drawY);
//    AEMtx33Concat(&transform, &trans, &scale);
//
//    // Set render mode and color
//    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
//    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
//    AEGfxSetColorToAdd(1.0f, 1.0f, 1.0f, 0.0f);
//
//    // Draw the character
//    AEGfxSetTransform(transform.m);
//    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);
//
//    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
//    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
//}
//
//void Character::SetPosition(int x, int y) {
//    gridX = x;
//    gridY = y;
//}