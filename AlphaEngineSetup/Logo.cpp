//author : Winson Teo
#include "GameStateManager.h"
#include "AEEngine.h"
#include "Utils.h"

#include <iostream>
#include <vector>
#include <string>

static AEGfxVertexList* pSquareMesh;
static AEGfxTexture* pLogo;
static f32 elapsedtime;
static s8 fontId = -1;

void Logo_Load() {
	if(pSquareMesh == nullptr) pSquareMesh = CreateSquare();
	pLogo = AEGfxTextureLoad("Assets/DigiPen_BLACK.png");
    fontId = AEGfxCreateFont("Assets/exo2-regular.ttf", 24);
}

void Logo_Init() {
	elapsedtime = 0.0f;
}

void Logo_Update() {
	f32 dt = (f32)AEFrameRateControllerGetFrameTime();
	elapsedtime += dt;

	if (elapsedtime >= 5.0f || AEInputCheckTriggered(AEVK_LBUTTON)) {
		elapsedtime = 0.0f;
		gGameStateNext = GS_MAINMENU;  
	}
}

void Logo_Draw() {
    AEGfxSetBackgroundColor(0.15f, 0.15f, 0.15f);
    AEGfxSetRenderMode(AE_GFX_RM_COLOR); // Start with a neutral mode
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f); // Grey so white logo is visible
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);

    // 2. THE TINT RESET (CRITICAL)
    // Multiply by 1.0 means "keep original color"
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    // Add 0.0 means "don't brighten/wash out"
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetTransparency(1.0f);

    // 3. Transformation
    AEMtx33 scale, trans, total;
    AEMtx33Scale(&scale, AEGfxGetWindowWidth() - 400.0f, AEGfxGetWindowHeight() - 400.0f);
    AEMtx33Trans(&trans, 0.0f, 0.0f);
    AEMtx33Concat(&total, &trans, &scale);
    AEGfxSetTransform(total.m);

    // 4. Bind and Draw
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

void Logo_Free(){}

void Logo_Unload() {
    AEGfxTextureUnload(pLogo);
    AEGfxDestroyFont(fontId);
    fontId = -1;

    if (pSquareMesh) {
        AEGfxMeshFree(pSquareMesh);
        pSquareMesh = nullptr;
    }
}