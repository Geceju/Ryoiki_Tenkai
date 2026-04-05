//author : Winson Teo
#include "GameStateManager.h"
#include "AEEngine.h"
#include "Utils.h"

#include <iostream>
#include <vector>
#include <string>

// === Assets & State ===
static AEGfxTexture* pLogo;
static s8 fontId = -1;
//static AEAudioGroup bgmGroup;
//static AEAudio bgm;

static f32 y_pos;
static f32 elapsedtime;
static int logo_done;
static f32 logo_y;
static int num_lines;
static AEGfxVertexList* pSquareMesh;

// === Credits list ===
static const std::vector<std::string> credits = {
    "===============================",
    "Ryoiki Tenkai",
    "===============================",
    "",
    "--- DEVELOPMENT TEAM ---",
    "Programmers:",
    "Winson Teo",
    "Tay Dylan",
    "Dylan Lim",
    "Felicia",
    "Keng Yip",
    "",
    "--- SPECIAL THANKS ---",
    "Claude Comair",
    "President, Founder and CEO of",
    "DigiPen Institute of Technology &",
    "DigiPen Game Studios",
    "",
    "--- EXECUTIVES ---",
    "CHU JASON YEU TAT",
    "TAN CHEK MING",
    "MICHAEL GATS",
    "PRASANNA KUMAR GHALI",
    "MANDY WONG",
    "JOHNNY DEEK",
    "WWW.DIGIPEN.EDU",
    "All content \xC2\xA9 2026 DigiPen Institute of Technology Singapore. All Rights Reserved",
    "",
    "--- MUSIC COPYRIGHTS ---",
    "Metadigger",
    "Soundly",
    "",
    "--- LECTURERS ---",
    "Prof Tommy",
    "Prof Soroor",
    "Prof Gerald",
    "",
    "--- TO THE PLAYER ---",
    "Thank you for playing!",
    "",
};

void Credits_Load() {
    if(pSquareMesh == nullptr) pSquareMesh = CreateSquare();
    pLogo = AEGfxTextureLoad("Assets/DigiPen_BLACK.png");
    fontId = AEGfxCreateFont("Assets/exo2-regular.ttf", 24);
}

void Credits_Init() {
    elapsedtime = 0.0f;
    logo_done = 0;

    y_pos = -1.2f;
    logo_y = 0.0f; // AE center is (0,0)
    num_lines = (int)credits.size();
}

void Credits_Update() {
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();
    elapsedtime += dt;

    if (AEInputCheckTriggered(AEVK_Q) || AEInputCheckTriggered(AEVK_ESCAPE)) {
        gGameStateNext = GS_MAINMENU;
    }

    if (!logo_done) {
        if (elapsedtime >= 2.0f){     
        
            logo_done = 1;
            elapsedtime = 0.0f;
        }
    }
    else {
        y_pos += 0.3f * dt;
        float line_spacing = 0.1f;
        if (y_pos - (num_lines * line_spacing) > 1.0f ) {
            gGameStateNext = GS_MAINMENU;
        }
    }
}

void Credits_Draw() {

    AEGfxSetBackgroundColor(0.15f, 0.15f, 0.15f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);

    if (!logo_done) {

        AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f); 
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetTransparency(1.0f);

        AEMtx33 scale, trans, total;
        AEMtx33Scale(&scale, AEGfxGetWindowWidth()-400.0f, AEGfxGetWindowHeight()-400.0f);
        AEMtx33Trans(&trans, 0.0f, 0.0f);
        AEMtx33Concat(&total, &trans, &scale);
        AEGfxSetTransform(total.m);

        if (pLogo != nullptr) {

            AEGfxTextureSet(pLogo, 0, 0);
            AEGfxMeshDraw(pSquareMesh, AE_GFX_MDM_TRIANGLES);
        }

        AEMtx33Identity(&total);
        AEGfxSetTransform(total.m);

        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        char copyright[] = "All content (c) 2026 DigiPen Institute";
        AEGfxPrint(fontId, copyright, -0.4f, -0.6f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {
        AEGfxSetBackgroundColor(0.0f, 0.0f, 0.0f);
        AEMtx33 total;
        AEMtx33Identity(&total);
        AEGfxSetTransform(total.m);
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);

        f32 line_spacing = 0.1f;
        for (int i = 0; i < num_lines; i++) {
            f32 current_y = y_pos - (i * line_spacing);

            if (current_y > -1.1f && current_y < 1.1f) {
                f32 alpha = 1.0f;
                if (current_y > 0.8f) alpha = (1.0f - current_y) / 0.2f;
                if (current_y < -0.8f) alpha = (current_y + 1.0f) / 0.2f;

                AEGfxPrint(fontId, credits[i].c_str(), -0.3f, current_y, 1.0f, 1.0f, 1.0f, 1.0f, alpha);
            }
        }
    }
}

void Credits_Free() {}

void Credits_Unload() {
    AEGfxTextureUnload(pLogo);
    AEGfxDestroyFont(fontId);
    fontId = -1;

    if (pSquareMesh) {
        AEGfxMeshFree(pSquareMesh);
        pSquareMesh = nullptr;
    }
}