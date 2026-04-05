// author : Tay Dylan
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "AABBCollision.h"
#include "AudioSystem.h"
#include <cstdio>
#include <cmath>

// Global volume and synchronization settings
float g_MusicVolume = 1.0f;
float g_SFXVolume = 1.0f;
bool g_VSyncEnabled = true;

struct UI_Button
{
    float x, y, scaleX, scaleY, r, g, b;
};

// 0 = none, 1 = exit to menu, 2 = exit game
static int g_ConfirmState = 0;
static UI_Button btnYes, btnNo;

// Button definitions for navigation and audio control
static UI_Button btnClose, btnVSync, btnExitMenu;
static UI_Button btnMusMinus, btnMusPlus;
static UI_Button btnSFXMinus, btnSFXPlus;

static AEGfxVertexList* pMeshSettings = nullptr;
static s8 g_FontSettings = -1;

void SettingsMenu_Load()
{
    // Create a unit square mesh for UI rendering
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMeshSettings = AEGfxMeshEnd();

    // Initialize font with a fallback to arial if the asset is missing
    g_FontSettings = AEGfxCreateFont("Assets/Exo2-Regular.ttf", 20);
    if (g_FontSettings < 0) g_FontSettings = AEGfxCreateFont("Arial", 20);

    g_ConfirmState = 0;
}

void SettingsMenu_Initialize()
{
    // Offset values to align grey boxes correctly on the right side

    // Close button (top right)
    btnClose = { 175.0f, 125.0f, 30.0f, 30.0f, 0.8f, 0.2f, 0.2f };

    // Music control button coordinates
    btnMusMinus = { 25.0f, 80.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };
    btnMusPlus = { 145.0f, 80.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };

    // Sfx control button coordinates
    btnSFXMinus = { 25.0f, 4.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };
    btnSFXPlus = { 145.0f, 4.0f, 30.0f, 30.0f, 0.4f, 0.4f, 0.4f };

    // Vsync toggle button coordinates
    btnVSync = { 85.0f, -65.0f, 100.0f, 40.0f, 0.4f, 0.4f, 0.4f };

    // Navigation button to return to the main menu
    btnExitMenu = { 0.0f, -130.0f, 240.0f, 40.0f, 0.8f, 0.2f, 0.2f };

    // Centered coordinates for the confirmation state overlay
    btnYes = { -60.0f, -50.0f, 80.0f, 40.0f, 0.0f, 1.0f, 0.0f };
    btnNo = { 60.0f, -50.0f, 80.0f, 40.0f, 1.0f, 0.0f, 0.0f };

    AEGfxSetVSync(g_VSyncEnabled ? 1 : 0);
}

void SettingsMenu_Update(bool& isMenuOpen)
{
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

    float winW = (float)AEGfxGetWindowWidth();
    float winH = (float)AEGfxGetWindowHeight();

    // Map screen-space mouse coordinates to world-space for AABB collision
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

        // Decrement music volume and update audio engine
        if (Collision_PointInButton(worldMX, worldMY, btnMusMinus.x + camX, btnMusMinus.y + camY, btnMusMinus.scaleX, btnMusMinus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_MusicVolume -= 0.1f;
            if (g_MusicVolume < 0.0f) g_MusicVolume = 0.0f;
            AudioSystem::SetBGMVolume(g_MusicVolume);
        }

        // Increment music volume and update audio engine
        if (Collision_PointInButton(worldMX, worldMY, btnMusPlus.x + camX, btnMusPlus.y + camY, btnMusPlus.scaleX, btnMusPlus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_MusicVolume += 0.1f;
            if (g_MusicVolume > 1.0f) g_MusicVolume = 1.0f;
            AudioSystem::SetBGMVolume(g_MusicVolume);
        }

        // Decrement sfx volume and update audio engine
        if (Collision_PointInButton(worldMX, worldMY, btnSFXMinus.x + camX, btnSFXMinus.y + camY, btnSFXMinus.scaleX, btnSFXMinus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_SFXVolume -= 0.1f;
            if (g_SFXVolume < 0.0f) g_SFXVolume = 0.0f;
            AudioSystem::SetSFXVolume(g_SFXVolume);
        }

        // Increment sfx volume and update audio engine
        if (Collision_PointInButton(worldMX, worldMY, btnSFXPlus.x + camX, btnSFXPlus.y + camY, btnSFXPlus.scaleX, btnSFXPlus.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_SFXVolume += 0.1f;
            if (g_SFXVolume > 1.0f) g_SFXVolume = 1.0f;
            AudioSystem::SetSFXVolume(g_SFXVolume);
        }

        // Toggle vertical sync state and log to console
        if (Collision_PointInButton(worldMX, worldMY, btnVSync.x + camX, btnVSync.y + camY, btnVSync.scaleX, btnVSync.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_VSyncEnabled = !g_VSyncEnabled;
            AEGfxSetVSync(g_VSyncEnabled ? 1 : 0);
            printf("VSync: %s\n", g_VSyncEnabled ? "ON" : "OFF");
        }

        // Transition to menu exit confirmation state
        if (Collision_PointInButton(worldMX, worldMY, btnExitMenu.x + camX, btnExitMenu.y + camY, btnExitMenu.scaleX, btnExitMenu.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            AudioSystem::Play("Click");
            g_ConfirmState = 1;
        }
    }
    else
    {
        // Confirm exit and transition back to main menu state
        if (Collision_PointInButton(worldMX, worldMY, btnYes.x + camX, btnYes.y + camY, btnYes.scaleX, btnYes.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Reset audio groups to prevent logic overlap on state change
            AudioSystem::StopGroup(false);
            AudioSystem::StopGroup(true);
            AudioSystem::Play("MenuBGM");

            AudioSystem::Play("Click");
            gGameStateNext = GS_MAINMENU;
        }

        // Revert confirmation state back to default settings view
        if (Collision_PointInButton(worldMX, worldMY, btnNo.x + camX, btnNo.y + camY, btnNo.scaleX, btnNo.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            g_ConfirmState = 0;
            AudioSystem::Play("Click");
        }
    }
}

static void DrawUIButtonRelative(const UI_Button& b, float camX, float camY)
{
    // Apply camera-relative translation to world-space button coordinates
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, b.scaleX, b.scaleY);
    AEMtx33Trans(&trans, b.x + camX, b.y + camY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetColorToMultiply(b.r, b.g, b.b, 1.0f);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);
}

void SettingsMenu_Draw(bool isIngame)
{
    float camX, camY;
    AEGfxGetCamPosition(&camX, &camY);

    // Draw background panel as an opaque layer to prevent ghosting
    AEMtx33 scale, trans, transform;
    AEMtx33Scale(&scale, 400.0f, 300.0f);
    AEMtx33Trans(&trans, camX, camY);
    AEMtx33Concat(&transform, &trans, &scale);

    AEGfxSetColorToMultiply(0.15f, 0.15f, 0.15f, 1.0f);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);

    // Render interactive buttons relative to camera position
    DrawUIButtonRelative(btnClose, camX, camY);
    DrawUIButtonRelative(btnMusMinus, camX, camY);
    DrawUIButtonRelative(btnMusPlus, camX, camY);
    DrawUIButtonRelative(btnSFXMinus, camX, camY);
    DrawUIButtonRelative(btnSFXPlus, camX, camY);
    DrawUIButtonRelative(btnVSync, camX, camY);
    if (isIngame) DrawUIButtonRelative(btnExitMenu, camX, camY);

    // Render text using normalized device coordinates
    if (g_FontSettings >= 0)
    {
        // Header and closing icon
        AEGfxPrint(g_FontSettings, (char*)"Settings", -0.039f, 0.27f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"Settings", -0.04f, 0.27f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"X", 0.21f, 0.26f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        char buf[32];
        // Music volume percentage display
        snprintf(buf, sizeof(buf), "Music: %d%%", (int)std::round(g_MusicVolume * 100));
        AEGfxPrint(g_FontSettings, buf, -0.2f, 0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"-", 0.0257f, 0.165f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"+", 0.175f, 0.165f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // Sfx volume percentage display
        snprintf(buf, sizeof(buf), "SFX: %d%%", (int)std::round(g_SFXVolume * 100));
        AEGfxPrint(g_FontSettings, buf, -0.174f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"-", 0.0257f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxPrint(g_FontSettings, (char*)"+", 0.175f, -0.005f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // Current vsync status
        AEGfxPrint(g_FontSettings, (char*)"VSync:", -0.13f, -0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        snprintf(buf, sizeof(buf), "%s", g_VSyncEnabled ? "ON" : "OFF");
        AEGfxPrint(g_FontSettings, buf, 0.09f, -0.16f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        if (isIngame)
        {
            AEGfxPrint(g_FontSettings, (char*)"Exit to Menu", -0.07f, -0.3f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }

    if (g_ConfirmState != 0)
    {
        // Render a semi-transparent modal overlay for the confirmation state
        AEMtx33 scalel, transl, transforml;
        AEMtx33Scale(&scalel, 400.0f, 300.0f);
        AEMtx33Trans(&transl, camX, camY);
        AEMtx33Concat(&transforml, &transl, &scalel);

        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.9f);
        AEGfxSetTransform(transforml.m);
        AEGfxMeshDraw(pMeshSettings, AE_GFX_MDM_TRIANGLES);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        DrawUIButtonRelative(btnYes, camX, camY);
        DrawUIButtonRelative(btnNo, camX, camY);

        // Confirmation text prompts
        if (g_FontSettings >= 0)
        {
            AEGfxPrint(g_FontSettings, (char*)"Exit to Menu?", -0.07f, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxPrint(g_FontSettings, (char*)"YES", -0.095f, -0.125f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxPrint(g_FontSettings, (char*)"NO", 0.06f, -0.125f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        }
    }
}

void SettingsMenu_Unload()
{
    // Clean up allocated graphics resources
    if (pMeshSettings) { AEGfxMeshFree(pMeshSettings); pMeshSettings = nullptr; }
    if (g_FontSettings >= 0) { AEGfxDestroyFont(g_FontSettings); g_FontSettings = -1; }
}