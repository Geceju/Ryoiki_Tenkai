#include "GameStateManager.h"
#include <iostream>
#include "AEEngine.h"

// Structure defining the visual and spatial properties of a menu button
typedef struct{
    float x, y;
    float scaleX, scaleY;
    float r, g, b;
} Button;

// Global button instances and the shared mesh used to render them
static Button btnPlay, btnExit;
static AEGfxVertexList* pMeshButton = 0;

void MainMenu_Load()
{
    // Define a 1x1 square mesh that will be scaled and moved to represent different buttons
    AEGfxMeshStart();

    // Construct the first triangle using counter-clockwise winding and standard UV mapping
    AEGfxTriAdd(
        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
    );

    // Construct the second triangle to finalize the square shape
    AEGfxTriAdd(
        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
    );

    pMeshButton = AEGfxMeshEnd();
}

void MainMenu_Initialize()
{
    // Configure Play Button (Green, Top)
    btnPlay.x = 0.0f;
    btnPlay.y = 50.0f;
    btnPlay.scaleX = 200.0f;
    btnPlay.scaleY = 80.0f;
    btnPlay.r = 0.0f; btnPlay.g = 1.0f; btnPlay.b = 0.0f;

    // Configure Exit Button (Red, Bottom)
    btnExit.x = 0.0f;
    btnExit.y = -50.0f;
    btnExit.scaleX = 200.0f;
    btnExit.scaleY = 80.0f;
    btnExit.r = 1.0f; btnExit.g = 0.0f; btnExit.b = 0.0f;
}

void MainMenu_Update()
{
    // Retrieve the current mouse position in screen coordinates
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    // Convert screen coordinates to world coordinates (assuming camera is at 0,0)
    // Screen (0,0) is top-left; World (0,0) is center.
    float worldMouseX = (float)mouseX - (AEGfxGetWindowWidth() / 2.0f);
    float worldMouseY = (float)(AEGfxGetWindowHeight() / 2.0f) - mouseY;

    // --- PLAY BUTTON LOGIC ---
    if (worldMouseX >= btnPlay.x - (btnPlay.scaleX / 2) &&
        worldMouseX <= btnPlay.x + (btnPlay.scaleX / 2) &&
        worldMouseY >= btnPlay.y - (btnPlay.scaleY / 2) &&
        worldMouseY <= btnPlay.y + (btnPlay.scaleY / 2))
    {
        btnPlay.g = 0.5f; // Hover effect (darker green)
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            gGameStateNext = GS_LEVEL1;
        }
    }
    else
    {
        btnPlay.g = 1.0f; // Reset to full green
    }

    // --- EXIT BUTTON LOGIC ---
    if (worldMouseX >= btnExit.x - (btnExit.scaleX / 2) &&
        worldMouseX <= btnExit.x + (btnExit.scaleX / 2) &&
        worldMouseY >= btnExit.y - (btnExit.scaleY / 2) &&
        worldMouseY <= btnExit.y + (btnExit.scaleY / 2))
    {
        btnExit.r = 0.5f; // Hover effect (darker red)
        if (AEInputCheckTriggered(AEVK_LBUTTON))
        {
            gGameStateNext = GS_QUIT;
        }
    }
    else {
        btnExit.r = 1.0f; // Reset to full red
    }
}

void MainMenu_Draw()
{
    // Clear background to a dark grey
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    AEMtx33 scale, trans, transform;

    // --- DRAW PLAY BUTTON ---
    AEGfxSetColorToMultiply(btnPlay.r, btnPlay.g, btnPlay.b, 1.0f);
    AEMtx33Scale(&scale, btnPlay.scaleX, btnPlay.scaleY);
    AEMtx33Trans(&trans, btnPlay.x, btnPlay.y);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetTransform(transform.m); // Use .m for the float[3][3] parameter
    AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);

    // --- DRAW EXIT BUTTON ---
    AEGfxSetColorToMultiply(btnExit.r, btnExit.g, btnExit.b, 1.0f);
    AEMtx33Scale(&scale, btnExit.scaleX, btnExit.scaleY);
    AEMtx33Trans(&trans, btnExit.x, btnExit.y);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
}

void MainMenu_Free()
{
    // Mesh is cleaned up in Unload, so we clear per-frame data here if needed
}

void MainMenu_Unload()
{
    if (pMeshButton)
    {
        AEGfxMeshFree(pMeshButton);
        pMeshButton = nullptr;
    }
}

//#include "GameStateManager.h"
//#include <stdio.h>
//#include "AEEngine.h"
//
//// Button Data
//typedef struct {
//    float x, y;       // Position
//    float scaleX, scaleY; // Size
//    float r, g, b;    // Color
//} Button;
//
//Button btnPlay, btnExit;
//AEGfxVertexList* pMeshButton = 0; // The mesh shape for buttons
//
//// --- LOAD: Create meshes/Load Textures ---
//void MainMenu_Load()
//{
//    // Create a simple 1x1 Quad Mesh for buttons
//    AEGfxMeshStart();
//    AEGfxTriAdd(
//        -0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
//        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//    );
//    AEGfxTriAdd(
//        0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//        0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
//        -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//    );
//    pMeshButton = AEGfxMeshEnd();
//}
//
//// --- INIT: Set variable values ---
//void MainMenu_Initialize()
//{
//    // Configure Play Button (Green, Top)
//    btnPlay.x = 0.0f; btnPlay.y = 50.0f;
//    btnPlay.scaleX = 200.0f; btnPlay.scaleY = 80.0f;
//    btnPlay.r = 0.0f; btnPlay.g = 1.0f; btnPlay.b = 0.0f;
//
//    // Configure Exit Button (Red, Bottom)
//    btnExit.x = 0.0f; btnExit.y = -50.0f;
//    btnExit.scaleX = 200.0f; btnExit.scaleY = 80.0f;
//    btnExit.r = 1.0f; btnExit.g = 0.0f; btnExit.b = 0.0f;
//}
//
//// --- UPDATE: Input and Logic ---
//void MainMenu_Update()
//{
//    // 1. Get Mouse Position
//    s32 mouseX, mouseY;
//    AEInputGetCursorPosition(&mouseX, &mouseY);
//
//    // Convert screen coordinates to world coordinates 
//    // (Assuming camera is at 0,0)
//    float worldMouseX = (float)mouseX - (AEGfxGetWindowWidth() / 2);
//    float worldMouseY = -((float)mouseY - (AEGfxGetWindowHeight() / 2)); // Flip Y for Alpha Engine
//
//    // 2. Check Input for PLAY Button
//    // Logic: Is Mouse X inside Left/Right bounds AND Mouse Y inside Top/Bottom bounds?
//    if (worldMouseX >= btnPlay.x - (btnPlay.scaleX / 2) &&
//        worldMouseX <= btnPlay.x + (btnPlay.scaleX / 2) &&
//        worldMouseY >= btnPlay.y - (btnPlay.scaleY / 2) &&
//        worldMouseY <= btnPlay.y + (btnPlay.scaleY / 2))
//    {
//        // Mouse is hovering Play
//        btnPlay.r = 0.5f; // Highlight effect
//
//        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
//            gGameStateNext = GS_PLAY; // SWITCH STATE
//        }
//    }
//    else {
//        btnPlay.r = 0.0f; // Reset color
//    }
//
//    // 3. Check Input for EXIT Button
//    if (worldMouseX >= btnExit.x - (btnExit.scaleX / 2) &&
//        worldMouseX <= btnExit.x + (btnExit.scaleX / 2) &&
//        worldMouseY >= btnExit.y - (btnExit.scaleY / 2) &&
//        worldMouseY <= btnExit.y + (btnExit.scaleY / 2))
//    {
//        btnExit.r = 0.5f; // Highlight
//
//        if (AEInputCheckTriggered(AEVK_LBUTTON)) {
//            gGameStateNext = GS_QUIT; // EXIT GAME
//        }
//    }
//    else {
//        btnExit.r = 1.0f; // Reset color
//    }
//}
//
//// --- DRAW: Render to screen ---
//void MainMenu_Draw()
//{
//    AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f);
//    // Define temporary matrices for calculation
//    AEMtx33 scale, trans, transform;
//
//    // Set Render Mode
//    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
//    // If textures are turned off, we use BlendColor or ColorToMultiply to set the color
//    AEGfxSetBlendColor(0.0f, 0.0f, 0.0f, 0.0f); // Reset blend
//
//    // --- DRAW PLAY BUTTON ---
//
//    // 1. Set Color (This replaces AEGfxSetTintColor)
//    // We use AEGfxSetColorToMultiply to tint the white mesh
//    AEGfxSetColorToMultiply(btnPlay.r, btnPlay.g, btnPlay.b, 1.0f);
//
//    // 2. Create Scale Matrix
//    AEMtx33Scale(&scale, btnPlay.scaleX, btnPlay.scaleY);
//
//    // 3. Create Translation (Position) Matrix
//    AEMtx33Trans(&trans, btnPlay.x, btnPlay.y);
//
//    // 4. Combine them: Transform = Translation * Scale
//    // (Note: The order is usually Trans * Rot * Scale)
//    AEMtx33Concat(&transform, &trans, &scale);
//
//    // 5. Send Matrix to Graphics Card and Draw
//    AEGfxSetTransform(transform.m);
//    AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
//
//
//    // --- DRAW EXIT BUTTON ---
//
//    // 1. Set Color
//    AEGfxSetColorToMultiply(btnExit.r, btnExit.g, btnExit.b, 1.0f);
//
//    // 2. Create Scale Matrix
//    AEMtx33Scale(&scale, btnExit.scaleX, btnExit.scaleY);
//
//    // 3. Create Translation Matrix
//    AEMtx33Trans(&trans, btnExit.x, btnExit.y);
//
//    // 4. Combine
//    AEMtx33Concat(&transform, &trans, &scale);
//
//    // 5. Draw
//    AEGfxSetTransform(transform.m);
//    AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
//}
//
//// --- FREE: Unload memory ---
//void MainMenu_Free()
//{
//    // Alpha Engine usually handles mesh freeing automatically or via specific calls
//    if (pMeshButton) AEGfxMeshFree(pMeshButton);
//}
//
//// --- UNLOAD ---
//void MainMenu_Unload()
//{
//    // Reset variables if needed
//}