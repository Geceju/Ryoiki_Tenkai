#include "MainMenu.h"
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "AEEngine.h"
#include "AABBCollision.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

struct Button {
    float x, y, scaleX, scaleY, r, g, b;
};

static Button btnPlay, btnSettings, btnExit;
static AEGfxVertexList* pMeshButton = nullptr;
static s8 g_FontIdMenu = -1;
static s8 g_FontIdTitle = -1;

static bool showSettingsMenu = false;

// Background animation
static bool bgIsRunning = false;
static float bgTimer = 0.0f;
static float bgPlayerX, bgPlayerY, bgEnemyX, bgEnemyY;
static float bgDirX, bgDirY, bgSpeed = 400.0f, bgTravelDist, bgMaxDist;
static float g_TotalTime = 0.0f;

void MainMenu_Load() {
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pMeshButton = AEGfxMeshEnd();

    g_FontIdMenu = AEGfxCreateFont("Assets/exo2-regular.ttf", 24);
    g_FontIdTitle = AEGfxCreateFont("Assets/exo2-regular.ttf", 96);

    SettingsMenu_Load();
}

void MainMenu_Initialize() {
    AEGfxSetCamPosition(0.0f, 0.0f);
    float bR = 0.2f, bG = 0.6f, bB = 0.8f;

    btnPlay = { 0.0f, 80.0f, 200.0f, 70.0f, bR, bG, bB };
    btnSettings = { 0.0f, -10.0f, 200.0f, 70.0f, bR, bG, bB };
    btnExit = { 0.0f, -100.0f, 200.0f, 70.0f, bR, bG, bB };

    showSettingsMenu = false;
    bgIsRunning = false;
    bgTimer = 0.0f;
    g_TotalTime = 0.0f;

    SettingsMenu_Initialize();
}

void MainMenu_Update() {
    float dt = (float)AEFrameRateControllerGetFrameTime();
    g_TotalTime += dt;

    if (showSettingsMenu) {
        SettingsMenu_Update(showSettingsMenu);
        return;
    }

    s32 mx, my;
    AEInputGetCursorPosition(&mx, &my);
    float winW = (float)AEGfxGetWindowWidth(), winH = (float)AEGfxGetWindowHeight();
    float worldMX = (float)mx - winW / 2.0f, worldMY = winH / 2.0f - (float)my;

    if (Collision_PointInButton(worldMX, worldMY, btnPlay.x, btnPlay.y, btnPlay.scaleX, btnPlay.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) gGameStateNext = GS_LEVEL1;
    if (Collision_PointInButton(worldMX, worldMY, btnSettings.x, btnSettings.y, btnSettings.scaleX, btnSettings.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) showSettingsMenu = true;
    if (Collision_PointInButton(worldMX, worldMY, btnExit.x, btnExit.y, btnExit.scaleX, btnExit.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON)) gGameStateNext = GS_QUIT;

    // Background logic
    if (!bgIsRunning) {
        bgTimer -= dt;
        if (bgTimer <= 0.0f) {
            bgIsRunning = true;
            float angle = (float)(std::rand() % 360) * 3.14159f / 180.0f;
            float spawnDist = sqrtf(winW * winW + winH * winH) / 2.0f + 150.0f;
            bgMaxDist = spawnDist * 2.1f;
            bgTravelDist = 0.0f;
            bgDirX = -cosf(angle); bgDirY = -sinf(angle);
            bgEnemyX = cosf(angle) * spawnDist; bgEnemyY = sinf(angle) * spawnDist;
            bgPlayerX = bgEnemyX + bgDirX * 80.0f; bgPlayerY = bgEnemyY + bgDirY * 80.0f;
        }
    }
    else {
        float step = bgSpeed * dt;
        bgPlayerX += bgDirX * step; bgPlayerY += bgDirY * step;
        bgEnemyX += bgDirX * step; bgEnemyY += bgDirY * step;
        bgTravelDist += step;
        if (bgTravelDist > bgMaxDist) { bgIsRunning = false; bgTimer = 0.0f; }
    }
}

void MainMenu_Draw() {
    AEGfxSetBackgroundColor(0.1f, 0.1f, 0.1f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEMtx33 s, r, t, final;

    // --- DRAW BACKGROUND CHASE SEQUENCE ---
    if (bgIsRunning) {
        float bP = fabsf(sinf(g_TotalTime * 15.0f)) * 10.0f;
        float bE = fabsf(sinf(g_TotalTime * 15.0f - 1.0f)) * 10.0f;
        AEMtx33Rot(&r, atan2f(bgDirY, bgDirX));
        AEMtx33Scale(&s, 40, 40);

        // Fleeing Player
        AEMtx33Trans(&t, bgPlayerX, bgPlayerY + bP);
        AEMtx33Concat(&final, &t, &r); AEMtx33Concat(&final, &final, &s);
        AEGfxSetColorToMultiply(0, 1, 1, 1); AEGfxSetTransform(final.m); AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);

        // Pursuing Enemy
        AEMtx33Trans(&t, bgEnemyX, bgEnemyY + bE);
        AEMtx33Concat(&final, &t, &r); AEMtx33Concat(&final, &final, &s);
        AEGfxSetColorToMultiply(1, 0, 0, 1); AEGfxSetTransform(final.m); AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
    }

    float winHalfH = (float)AEGfxGetWindowHeight() / 2.0f;

    // Restored centering logic within the button drawing helper
    auto DrawBtn = [&](const Button& b, const char* text, float txOffset) {
        if (g_FontIdMenu < 0) return; // SAFEGUARD: Don't draw if font is destroyed!

        AEMtx33Scale(&s, b.scaleX, b.scaleY);
        AEMtx33Trans(&t, b.x, b.y);
        AEMtx33Concat(&final, &t, &s);
        AEGfxSetColorToMultiply(b.r, b.g, b.b, 1);
        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);

        float textY = (b.y / winHalfH) - 0.015f;
        AEGfxPrint(g_FontIdMenu, (char*)text, txOffset, textY, 1, 0, 0, 0, 1);
        };

    // restored specific horizontal offsets for perfect centering
    DrawBtn(btnPlay, "Start", -0.035f);
    DrawBtn(btnSettings, "Settings", -0.055f); // balanced offset for "Settings"
    DrawBtn(btnExit, "Exit", -0.025f);

    // Title centered at your tested X coordinate
    if (g_FontIdTitle >= 0) {
        AEGfxPrint(g_FontIdTitle, (char*)"Enoki Tenkai", -0.35f, 0.6f, 1, 1, 1, 1, 1);
    }

    if (showSettingsMenu) {
        SettingsMenu_Draw(false);
    }
}

void MainMenu_Free() {}

void MainMenu_Unload() {
    if (pMeshButton) {
        AEGfxMeshFree(pMeshButton);
        pMeshButton = nullptr; // Mark as dead
    }
    if (g_FontIdMenu >= 0) {
        AEGfxDestroyFont(g_FontIdMenu);
        g_FontIdMenu = -1; // Mark as dead
    }
    if (g_FontIdTitle >= 0) {
        AEGfxDestroyFont(g_FontIdTitle);
        g_FontIdTitle = -1; // Mark as dead
    }
    SettingsMenu_Unload();
}