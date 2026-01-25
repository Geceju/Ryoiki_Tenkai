#include "GameStateManager.h"
#include <iostream>
#include <array>
#include "AEEngine.h"


AEGfxVertexList* pWallMesh = 0; // The mesh shape for buttons
int TileSize = 48;
std::array<std::array<int, 20>, 15> maze = { {
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1},
    {1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1},
    {1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1},
    {1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1},
    {1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1},
    {1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1},
    {1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 1},
    {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 0, 0, 1},
    {1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 0, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 1, 0, 1},
    {1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
} };
void Level1_Load() {
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
    pWallMesh = AEGfxMeshEnd();
}
void Level1_Initialize() {

}
void Level1_Update() {

}
void Level1_Draw() {
    for (int y = 0; y < 15; ++y) {
        for (int x = 0; x < 20; ++x) {
            if (maze[y][x] == 1) {
                AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
                // Define temporary matrices for calculation
                AEMtx33 scale, trans, transform;

                // Set Render Mode
                AEGfxSetRenderMode(AE_GFX_RM_COLOR);
                // If textures are turned off, we use BlendColor or ColorToMultiply to set the color
                AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 1.0f);
                // 2. Calculate World Position
                // Offset by -WINDOW_WIDTH/2 if your 0,0 is at the screen center
                float drawX = (x * TileSize) - (f32)AEGfxGetWindowWidth() / 2.0f + (TileSize / 2.0f);
                float drawY = (y * TileSize) - (f32)AEGfxGetWindowHeight() / 2.0f + (TileSize / 2.0f);

                // 3. Build the Transformation Matrix
                AEMtx33Scale(&scale, TileSize, TileSize);
                AEMtx33Trans(&trans, drawX, drawY);

                // Multiply them: transform = trans * rot * scale
                AEMtx33Concat(&transform, &trans, &scale);

                // 4. Send the matrix to Alpha Engine and Draw
                AEGfxSetTransform(transform.m);
                AEGfxMeshDraw(pWallMesh, AE_GFX_MDM_TRIANGLES);
            }
        }
    }
}
void Level1_Free() {

}
void Level1_Unload() {

}