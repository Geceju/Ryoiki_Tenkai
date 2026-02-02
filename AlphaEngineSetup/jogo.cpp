#include "jogo.h"
#include "items.h"  // NEW: Include for ItemsManager
#include <iostream>

Character::Character(int startX, int startY, int tile)
    : gridX(startX), gridY(startY), tileSize(tile), pMesh(nullptr),
    moveTimer(0.0f), moveDelay(0.10f) {
}

Character::~Character() {
    Unload();
}

void Character::Load() {
    // Create a white square mesh
    AEGfxMeshStart();

    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
    );

    AEGfxTriAdd(
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
    );

    pMesh = AEGfxMeshEnd();
}

void Character::Unload() {
    if (pMesh) {
        AEGfxMeshFree(pMesh);
        pMesh = nullptr;
    }
}

bool Character::IsValidPosition(int x, int y, const std::array<std::array<int, 20>, 15>& maze) {
    // Check bounds
    if (x < 0 || x >= 20 || y < 0 || y >= 15) {
        return false;
    }

    // Check if it's not a wall (0 = no wall, 1 = wall)
    return maze[y][x] == 0;
}

void Character::Update(const std::array<std::array<int, 20>, 15>& maze) {
    // Update timer
    moveTimer += (f32)AEFrameRateControllerGetFrameTime();

    // Only allow movement if enough time has passed
    if (moveTimer >= moveDelay) {
        int newX = gridX;
        int newY = gridY;
        bool moved = false;

        // Check for key presses and calculate new position
        if (AEInputCheckCurr(AEVK_W) || AEInputCheckCurr(AEVK_UP)) {
            newY += 1;
            moved = true;
        }
        else if (AEInputCheckCurr(AEVK_S) || AEInputCheckCurr(AEVK_DOWN)) {
            newY -= 1;
            moved = true;
        }

        if (AEInputCheckCurr(AEVK_A) || AEInputCheckCurr(AEVK_LEFT)) {
            newX -= 1;
            moved = true;
        }
        else if (AEInputCheckCurr(AEVK_D) || AEInputCheckCurr(AEVK_RIGHT)) {
            newX += 1;
            moved = true;
        }

        // Only update position if the new position is valid (not a wall)
        if (moved && IsValidPosition(newX, newY, maze)) {
            gridX = newX;
            gridY = newY;
            moveTimer = 0.0f;
        }
    }
}

// ========== NEW FUNCTION ADDED ==========
void Character::CollectItem(ItemsManager& itemsManager) {
    // Check if E key is pressed (triggered once per press)
    if (AEInputCheckTriggered(AEVK_E)) {
        // Get character's world position
        float playerX = GetWorldX();
        float playerY = GetWorldY();

        // Call the Update method which checks for collection
        // We pass 0.0f for deltaTime since we're just checking collection
        itemsManager.Update(playerX, playerY, 0.0f);

        std::cout << "Player attempted to collect item at position ("
            << playerX << ", " << playerY << ")\n";
    }
}
// =========================================

void Character::Draw() {
    if (!pMesh) return;

    // Calculate world position from grid position
    float drawX = (gridX * tileSize) - (f32)AEGfxGetWindowWidth() / 2.0f + (tileSize / 2.0f);
    float drawY = (gridY * tileSize) - (f32)AEGfxGetWindowHeight() / 2.0f + (tileSize / 2.0f);

    // Create transformation matrices
    AEMtx33 scale, trans, transform;

    // Scale to tile size
    AEMtx33Scale(&scale, (f32)(tileSize) * 0.8f, (f32)(tileSize) * 0.8f);
    AEMtx33Trans(&trans, drawX, drawY);
    AEMtx33Concat(&transform, &trans, &scale);

    // Set render mode and color
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(1.0f, 1.0f, 1.0f, 0.0f);

    // Draw the character
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMesh, AE_GFX_MDM_TRIANGLES);

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
}

void Character::SetPosition(int x, int y) {
    gridX = x;
    gridY = y;
}