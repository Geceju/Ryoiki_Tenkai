#include "MainMenu.h"
#include "SettingsMenu.h"
#include "GameStateManager.h"
#include "AEEngine.h"
#include "AABBCollision.h"
#include "Leaderboard.h"
#include "AudioSystem.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

struct Button {
    float x, y, scaleX, scaleY, r, g, b;
};

static Button btnPlay, btnSettings, btnLeaderboard, btnExit;
static AEGfxVertexList* pMeshButton = nullptr;
static s8 g_FontIdMenu = -1;
static s8 g_FontIdTitle = -1;

static AEGfxTexture* pBgPlayerTexture = nullptr;
static AEGfxTexture* pBgEnemyTexture = nullptr;

static bool showSettingsMenu = false;
static bool showLeaderboard = false;

static bool g_ShowQuitConfirm = false;
static Button btnQuitYes, btnQuitNo;

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
    pBgPlayerTexture = AEGfxTextureLoad("Assets/jogo.png");
    pBgEnemyTexture = AEGfxTextureLoad("Assets/enemy.png");

    SettingsMenu_Load();
}

void MainMenu_Initialize() {
    AEGfxSetCamPosition(0.0f, 0.0f);
    float bR = 0.2f, bG = 0.6f, bB = 0.8f;

    //btnPlay = { 0.0f, 80.0f, 200.0f, 70.0f, bR, bG, bB };
    //btnSettings = { 0.0f, -10.0f, 200.0f, 70.0f, bR, bG, bB };
    //btnExit = { 0.0f, -100.0f, 200.0f, 70.0f, bR, bG, bB };

    btnPlay = { 0.0f, 100.0f, 200.0f, 60.0f, bR, bG, bB };
    btnSettings = { 0.0f, 20.0f, 200.0f, 60.0f, bR, bG, bB };
    btnLeaderboard = { 0.0f, -60.0f, 200.0f, 60.0f, bR, bG, bB };
    btnExit = { 0.0f, -140.0f, 200.0f, 60.0f, bR, bG, bB };

    btnQuitYes = { -75.0f, -100.0f, 100.0f, 40.0f, 0.0f, 1.0f, 0.0f };
    btnQuitNo = { 75.0f, -100.0f, 100.0f, 40.0f, 1.0f, 0.0f, 0.0f };

    showSettingsMenu = false;
    showLeaderboard = false;
    bgIsRunning = false;
    g_ShowQuitConfirm = false;
    bgTimer = 0.0f;
    g_TotalTime = 0.0f;

    SettingsMenu_Initialize();
    LeaderboardSystem::Load();

    // Start the menu background music
    AudioSystem::Play("MenuBGM");
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

    // If leaderboard is open, click anywhere to close it
    if (showLeaderboard) {
        if (AEInputCheckTriggered(AEVK_LBUTTON) || AEInputCheckTriggered(AEVK_ESCAPE))
        {
            // Play a click sound effect
            AudioSystem::Play("Click");
            showLeaderboard = false;
        }
        return; // Stop updating main menu buttons
    }

    if (!g_ShowQuitConfirm) {
        if (Collision_PointInButton(worldMX, worldMY, btnPlay.x, btnPlay.y, btnPlay.scaleX, btnPlay.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Stop the BGM group (true = music group)
            AudioSystem::StopGroup(true);

            // Play a click sound effect
            AudioSystem::Play("Click");

            gGameStateNext = GS_LEVEL1;
        }
        if (Collision_PointInButton(worldMX, worldMY, btnSettings.x, btnSettings.y, btnSettings.scaleX, btnSettings.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Play a click sound effect
            AudioSystem::Play("Click");

            showSettingsMenu = true;
        }
        if (Collision_PointInButton(worldMX, worldMY, btnLeaderboard.x, btnLeaderboard.y, btnLeaderboard.scaleX, btnLeaderboard.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Play a click sound effect
            AudioSystem::Play("Click");

            showLeaderboard = true;
        }

        if (Collision_PointInButton(worldMX, worldMY, btnExit.x, btnExit.y, btnExit.scaleX, btnExit.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            // Play a click sound effect
            AudioSystem::Play("Click");
            g_ShowQuitConfirm = true;
        }
    }
    else {
        // Quit confirmation buttons
        if (Collision_PointInButton(worldMX, worldMY, btnQuitYes.x, btnQuitYes.y, btnQuitYes.scaleX, btnQuitYes.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            AudioSystem::Play("Click");
            gGameStateNext = GS_QUIT;
        }
        if (Collision_PointInButton(worldMX, worldMY, btnQuitNo.x, btnQuitNo.y, btnQuitNo.scaleX, btnQuitNo.scaleY) && AEInputCheckTriggered(AEVK_LBUTTON))
        {
            AudioSystem::Play("Click");
            g_ShowQuitConfirm = false;
        }
	}

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

        // 1. Calculate horizontal flip: 1.0 is right, -1.0 is left
        float flipX = (bgDirX >= 0) ? 1.0f : -1.0f;

        // 2. Fix the Rotation: Keep it 0.0f so they stay right-side up
        AEMtx33Rot(&r, 0.0f);

        // --- Fleeing Player ---
        // Apply flipX to the Scale matrix
        AEMtx33Scale(&s, 70.0f * flipX, 70.0f);
        AEMtx33Trans(&t, bgPlayerX, bgPlayerY + bP);
        AEMtx33Concat(&final, &t, &r);
        AEMtx33Concat(&final, &final, &s);

        if (pBgPlayerTexture != nullptr) {
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxSetTransparency(1.0f);
            AEGfxTextureSet(pBgPlayerTexture, 0.0f, 0.0f);
        }

        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);

        // --- Pursuing Enemy ---
        // Re-apply scale with flipX for the enemy
        AEMtx33Scale(&s, 70.0f * flipX, 70.0f);
        AEMtx33Trans(&t, bgEnemyX, bgEnemyY + bE);
        AEMtx33Concat(&final, &t, &r);
        AEMtx33Concat(&final, &final, &s);

        if (pBgEnemyTexture != nullptr) {
            AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
            AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
            AEGfxSetBlendMode(AE_GFX_BM_BLEND);
            AEGfxTextureSet(pBgEnemyTexture, 0.0f, 0.0f);
        }

        AEGfxSetTransform(final.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);

        // Reset state
        AEGfxTextureSet(nullptr, 0, 0);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);
    }

    float winHalfH = (float)AEGfxGetWindowHeight() / 2.0f;

    // Centering logic within the button drawing helper
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

    // Specific horizontal offsets for perfect centering
    DrawBtn(btnPlay, "Start", -0.035f);
    DrawBtn(btnSettings, "Settings", -0.055f); // balanced offset for "Settings"
    DrawBtn(btnLeaderboard, "Scores", -0.045f); // Draw new button
    DrawBtn(btnExit, "Exit", -0.025f);

    // Title centered at tested X coordinate
    if (g_FontIdTitle >= 0) {
        AEGfxPrint(g_FontIdTitle, (char*)"Enoki Tenkai", -0.35f, 0.6f, 1, 1, 1, 1, 1);
    }

    if (showSettingsMenu) {
        SettingsMenu_Draw(false);
    }

	// Leaderboard Overlay
    if (showLeaderboard && g_FontIdMenu >= 0){
        // Draw dark background panel
        AEMtx33 scale, trans, transform;
        AEMtx33Scale(&scale, 800.0f, 600.0f);
        AEMtx33Trans(&trans, 0, 0);
        AEMtx33Concat(&transform, &trans, &scale);

        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.9f);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        AEGfxPrint(g_FontIdTitle, (char*)"TOP 10 RUNS", -0.28f, 0.35f, 0.8f, 1.0f, 0.8f, 0.0f, 1.0f);

        const auto& runs = LeaderboardSystem::GetTopRuns();
        float startY = 0.15f;

        if (runs.empty()) {
            AEGfxPrint(g_FontIdMenu, (char*)"No runs logged yet!", -0.15f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f);
        }
        else {
            // Draw Headers once
            AEGfxPrint(g_FontIdMenu, (char*)"Rank", -0.35f, 0.20f, 0.8f, 0.5f, 0.5f, 0.5f, 1.0f);
            AEGfxPrint(g_FontIdMenu, (char*)"Name", -0.15f, 0.20f, 0.8f, 0.5f, 0.5f, 0.5f, 1.0f);
            AEGfxPrint(g_FontIdMenu, (char*)"Level | Time", 0.15f, 0.20f, 0.8f, 0.5f, 0.5f, 0.5f, 1.0f);

            for (size_t i = 0; i < runs.size(); ++i) {
                int minutes = static_cast<int>(runs[i].timeTaken) / 60;
                float seconds = fmod(runs[i].timeTaken, 60.0f);

                // Color top 3 yellow, rest white
                float r = (i < 3) ? 1.0f : 1.0f;
                float g = (i < 3) ? 1.0f : 1.0f;
                float b = (i < 3) ? 0.0f : 1.0f;

                // FIX: Shrunk the vertical gap from 0.08f to 0.06f so all 10 fit!
                float rowY = 0.05f - (i * 0.06f);

                // FIX: Dropped the text scale from 1.0f to 0.85f so they don't overlap vertically
                // 1. Draw Rank 
                char rankBuf[16];
                sprintf_s(rankBuf, "%d.", (int)i + 1);
                AEGfxPrint(g_FontIdMenu, rankBuf, -0.35f, rowY, 0.85f, r, g, b, 1.0f);

                // 2. Draw Username 
                char nameBuf[16];
                sprintf_s(nameBuf, "%s", runs[i].playerName.c_str());
                AEGfxPrint(g_FontIdMenu, nameBuf, -0.15f, rowY, 0.85f, r, g, b, 1.0f);

                // 3. Draw Time & Level 
                char timeBuf[32];
                sprintf_s(timeBuf, "Lvl %d  |  %02d:%05.2f", runs[i].levelReached, minutes, seconds);
                AEGfxPrint(g_FontIdMenu, timeBuf, 0.15f, rowY, 0.85f, r, g, b, 1.0f);
            }
        }
    }

    if (g_ShowQuitConfirm) {
        // Draw dark background panel
        AEMtx33 scale, trans, transform;
        AEMtx33Scale(&scale, 800.0f, 600.0f);
        AEMtx33Trans(&trans, 0, 0);
        AEMtx33Concat(&transform, &trans, &scale);

        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.9f); // 90% opacity black
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
        AEGfxSetBlendMode(AE_GFX_BM_NONE);

        // Draw Confirmation Text
        AEGfxPrint(g_FontIdMenu, (char*)"Are you sure you want to quit?", -0.2f, 0.1f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // Draw YES Button
        AEMtx33Scale(&scale, btnQuitYes.scaleX, btnQuitYes.scaleY);
        AEMtx33Trans(&trans, btnQuitYes.x, btnQuitYes.y);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetColorToMultiply(btnQuitYes.r, btnQuitYes.g, btnQuitYes.b, 1.0f);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
        AEGfxPrint(g_FontIdMenu, (char*)"YES", -0.118f, -0.24f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);

        // Draw NO Button
        AEMtx33Scale(&scale, btnQuitNo.scaleX, btnQuitNo.scaleY);
        AEMtx33Trans(&trans, btnQuitNo.x, btnQuitNo.y);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetColorToMultiply(btnQuitNo.r, btnQuitNo.g, btnQuitNo.b, 1.0f);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(pMeshButton, AE_GFX_MDM_TRIANGLES);
        AEGfxPrint(g_FontIdMenu, (char*)"NO", 0.075f, -0.24f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
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
    if (pBgPlayerTexture) {
        AEGfxTextureUnload(pBgPlayerTexture);
        pBgPlayerTexture = nullptr;
    }
    if (pBgEnemyTexture) {
        AEGfxTextureUnload(pBgEnemyTexture);
        pBgEnemyTexture = nullptr;
    }
    SettingsMenu_Unload();
}

//if (runs.empty()) {
//    AEGfxPrint(g_FontIdMenu, (char*)"No runs logged yet!", -0.143f, 0.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f);
//}
//else {
//    for (size_t i = 0; i < runs.size(); ++i) {
//        // Format the time into Minutes : Seconds
//        int minutes = static_cast<int>(runs[i].timeTaken) / 60;
//        float seconds = fmod(runs[i].timeTaken, 60.0f);

//        char buffer[64];
//        sprintf_s(buffer, "%d. Level %d  -  %02d:%05.2f", (int)i + 1, runs[i].levelReached, minutes, seconds);

//        // Color top 3 yellow, rest white
//        float r = (i < 3) ? 1.0f : 1.0f;
//        float g = (i < 3) ? 1.0f : 1.0f;
//        float b = (i < 3) ? 0.0f : 1.0f;

//        AEGfxPrint(g_FontIdMenu, buffer, -0.14f, startY - (i * 0.08f), 1.0f, r, g, b, 1.0f);
//    }
//}
//AEGfxPrint(g_FontIdMenu, (char*)"Click anywhere to close", -0.095f, 0.25f, 0.6f, 0.5f, 0.5f, 0.5f, 1.0f);