//author : Tay Dylan
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "AABBCollision.h"
#include "AudioSystem.h"
#include <cstdio>
#include <cmath>

// Global definitions
float g_MusicVolume = 1.0f;
float g_SFXVolume = 1.0f;
bool g_VSyncEnabled = true;

struct UI_Button {
    float x, y, scaleX, scaleY, r, g, b;
};

// 0 = None, 1 = Exit to Menu, 2 = Exit Game
static int g_ConfirmState = 0;
static UI_Button btnYes, btnNo;

// Add this to your SettingsMenu_Init or Load to position the confirm buttons


static UI_Button btnClose, btnVSync, btnExitMenu;
static UI_Button btnMusMinus, btnMusPlus;
static UI_Button btnSFXMinus, btnSFXPlus;

static AEGfxVertexList* pMeshSettings = nullptr;
static s8 g_FontSettings = -1;

void SettingsMenu_Load() {
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMeshSettings = AEGfxMeshEnd();

    // Use the exact case-sensitive filename from assets
    g_FontSettings = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 20);
    if (g_FontSettings < 0) g_FontSettings = AEGfxCreateFont("Arial", 20);

    g_ConfirmState = 0;
}

void SettingsMenu_Initialize() {
    // Current analysis: Grey boxes are too far left. 
    // Shifting them RIGHT (increasing X values).

    // Close Button (Top Right)
    btnClose = { 175.0f, 125.0f, 30.0f, 30.0f, 0.8f, 0.2f, 0.2f };

    // Music Row Buttons
    // Shifted RIGHT from 10.0f/65.0f to 25.0f/145.0f
    btnMusMinus = { 25.0f, 80.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };
    btnMusPlus = { 145.0f, 80.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };

    // SFX Row Buttons
    // Shifted RIGHT to match the Music row alignment
    btnSFXMinus = { 25.0f, 4.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };
    btnSFXPlus = { 145.0f, 4.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };

    // VSync Button (Shifted RIGHT and slightly DOWN)
    btnVSync = { 85.0f, -65.0f, 100.0f, 40.0f, 0.4f, 0.4f, 0.4f };

    // Exit to Menu Button
    btnExitMenu = { 0.0f, -130.0f, 240.0f, 40.0f, 0.8f, 0.2f, 0.2f };

    // Confirmation Buttons (Centered relative to menu)
    btnYes = { -60.0f, -50.0f, 80.0f, 40.0f, 0.0f, 1.0f, 0.0f };
    btnNo = { 60.0f, -50.0f, 80.0f, 40.0f, 1.0f, 0.0f, 0.0f };

    AEGfxSetVSync(g_VSyncEnabled ? 1 : 0);
}

void SettingsMenu_Update(bool& isMenuOpen) {
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

    float winW = (float)AEGfxGetWindowWidth();
    float winH = (float)AEGfxGetWindowHeight();

    // Convert screen mouse coordinates to world coordinates
    float worldMX = (float)mouseX - (winW / 2.0f) + camX;
    float worldMY = (winH / 2.0f) - (float)mouseY + camY;

    if (g_ConfirmState == 0)
    {
        // Close the settings overlay
        if (Collision_PointInButton(worldMX, worldMY, btnClose.x + camX, btnClose.y + camY, btnClose.scaleX, btnClose.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            AudioSystem::Play("Click");
            isMenuOpen = false;
        }

        // Music Volume Down
        if (Collision_PointInButton(worldMX, worldMY, btnMusMinus.x + camX, btnMusMinus.y + camY, btnMusMinus.scaleX, btnMusMinus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_MusicVolume -= 0.1f;
            if (g_MusicVolume < 0.0f) g_MusicVolume = 0.0f;
            // Update the actual audio engine group volume
            AudioSystem::SetBGMVolume(g_MusicVolume);
        }

        // Music Volume Up
        if (Collision_PointInButton(worldMX, worldMY, btnMusPlus.x + camX, btnMusPlus.y + camY, btnMusPlus.scaleX, btnMusPlus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_MusicVolume += 0.1f;
            if (g_MusicVolume > 1.0f) g_MusicVolume = 1.0f;
            // Update the actual audio engine group volume
            AudioSystem::SetBGMVolume(g_MusicVolume);
        }

        // SFX Volume Down
        if (Collision_PointInButton(worldMX, worldMY, btnSFXMinus.x + camX, btnSFXMinus.y + camY, btnSFXMinus.scaleX, btnSFXMinus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_SFXVolume -= 0.1f;
            if (g_SFXVolume < 0.0f) g_SFXVolume = 0.0f;
            // Update the actual audio engine group volume
            AudioSystem::SetSFXVolume(g_SFXVolume);
        }

        // SFX Volume Up
        if (Collision_PointInButton(worldMX, worldMY, btnSFXPlus.x + camX, btnSFXPlus.y + camY, btnSFXPlus.scaleX, btnSFXPlus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_SFXVolume += 0.1f;
            if (g_SFXVolume > 1.0f) g_SFXVolume = 1.0f;
            // Update the actual audio engine group volume
            AudioSystem::SetSFXVolume(g_SFXVolume);
        }

        // VSync Toggle
        if (Collision_PointInButton(worldMX, worldMY, btnVSync.x + camX, btnVSync.y + camY, btnVSync.scaleX, btnVSync.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_VSyncEnabled = !g_VSyncEnabled;
            AEGfxSetVSync(g_VSyncEnabled ? 1 : 0);
            printf("VSync: %s\n", g_VSyncEnabled ? "ON" : "OFF");
        }

        // Exit to Main Menu
        if (Collision_PointInButton(worldMX, worldMY, btnExitMenu.x + camX, btnExitMenu.y + camY, btnExitMenu.scaleX, btnExitMenu.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            AudioSystem::Play("Click");
            g_ConfirmState = 1; // Trigger "Exit to Menu?"
        }
    }
    else
    {
		// Exit to Main Menu confirmation
        if (Collision_PointInButton(worldMX, worldMY, btnYes.x + camX, btnYes.y + camY, btnYes.scaleX, btnYes.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Force silence for the game world
            AudioSystem::StopGroup(false);

            // Ensure MenuBGM starts fresh
            // Stop the Music group first to prevent overlapping
            AudioSystem::StopGroup(true);
            AudioSystem::Play("MenuBGM");

            AudioSystem::Play("Click");
            gGameStateNext = GS_MAINMENU;
        }
        if (Collision_PointInButton(worldMX, worldMY, btnNo.x + camX, btnNo.y + camY, btnNo.scaleX, btnNo.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) {
            g_ConfirmState = 0; // Back to settings
            AudioSystem::Play("Click");
        }
    }
}

static void DrawUIButtonRelative(const UI_Button& b, float camX, float camY) {
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, b.scaleX, b.scaleY);
    AEMtx33Trans(&trans, b.x + camX, b.y + camY);
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetColorToMultiply(b.r, b.g, b.b, 1.0f);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);
}

void SettingsMenu_Draw(bool isIngame) {
    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

    // --- DRAW BACKGROUND PANEL ---
    // Ensure alpha is 1.0f to act as a "clear" for the UI area and prevent ghosting
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 400.0f, 300.0f);
    AEMtx33Trans(&trans, camX, camY);
    AEMtx33Concat(&transform, &trans, &scale);
    AEGfxSetColorToMultiply(0.15f, 0.15f, 0.15f, 1.0f); // Fully opaque
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);

    // --- DRAW BUTTONS (Relative to Camera) ---
    DrawUIButtonRelative(btnClose, camX, camY);
    DrawUIButtonRelative(btnMusMinus, camX, camY);
    DrawUIButtonRelative(btnMusPlus, camX, camY);
    DrawUIButtonRelative(btnSFXMinus, camX, camY);
    DrawUIButtonRelative(btnSFXPlus, camX, camY);
    DrawUIButtonRelative(btnVSync, camX, camY);
    if (isIngame) DrawUIButtonRelative(btnExitMenu, camX, camY);

    // --- DRAW TEXT (NDC Coordinates) ---
    if (g_FontSettings >= 0) {
        AEGfxPrint(g_FontSettings, (char*)"Settings", -0.039f, 0.27f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"Settings", -0.04f, 0.27f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"X", 0.21f, 0.26f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        char buf[32];
        // Music Row
        snprintf(buf, sizeof(buf), "Music: %d%%", (int)std::round(g_MusicVolume * 100));
        AEGfxPrint(g_FontSettings, buf, -0.2f, 0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"-", 0.0257f, 0.165f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"+", 0.175f, 0.165f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // SFX Row
        snprintf(buf, sizeof(buf), "SFX: %d%%", (int)std::round(g_SFXVolume * 100));
        AEGfxPrint(g_FontSettings, buf, -0.174f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"-", 0.0257f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"+", 0.175f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // VSync Row
        AEGfxPrint(g_FontSettings, (char*)"VSync:", -0.13f, -0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        snprintf(buf, sizeof(buf), "%s", g_VSyncEnabled ? "ON" : "OFF");
        AEGfxPrint(g_FontSettings, buf, 0.09f, -0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        if (isIngame) {
            AEGfxPrint(g_FontSettings, (char*)"Exit to Menu", -0.07f, -0.3f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    if (g_ConfirmState != 0) {
        // Draw dark background panel (Relative to Camera)
        AEMtx33 scalel, transl, transforml;
        AEMtx33Scale(&scalel, 400.0f, 300.0f);
        AEMtx33Trans(&transl, camX, camY);
        AEMtx33Concat(&transforml, &transl, &scalel);

        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.9f);
        AEGfxSetTransform(transforml.m);
        AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        // Draw YES/NO Buttons using your existing helper
        DrawUIButtonRelative(btnYes, camX, camY);
        DrawUIButtonRelative(btnNo, camX, camY);

        // Draw Text
        if (g_FontSettings >= 0) {
            AEGfxPrint(g_FontSettings, (char*)"Exit to Menu?", -0.07f, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxPrint(g_FontSettings, (char*)"YES", -0.095f, -0.125f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxPrint(g_FontSettings, (char*)"NO", 0.06f, -0.125f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void SettingsMenu_Unload() {
    if (pMeshSettings) { AEGfxMeshFree(pMeshSettings); pMeshSettings = nullptr; }
    if (g_FontSettings >= 0) { AEGfxDestroyFont(g_FontSettings); g_FontSettings = -1; }
}