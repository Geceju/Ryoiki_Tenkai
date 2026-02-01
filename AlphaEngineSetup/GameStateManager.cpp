#include "AEEngine.h"
#include "GameStateManager.h"
#include "MainMenu.h"
#include "Level1.h"
#include "AEInput.h"

// Global State Variables
GAME_STATE gGameStateCurr = GS_NONE;
GAME_STATE gGameStateNext = GS_NONE;
GAME_STATE gGameStatePrev = GS_NONE;

// Function Pointers
void (*GameStateLoad)() = 0;
void (*GameStateInit)() = 0;
void (*GameStateUpdate)() = 0;
void (*GameStateDraw)() = 0;
void (*GameStateFree)() = 0;
void (*GameStateUnload)() = 0;

void GSM_Initialize(GAME_STATE startState)
{
	gGameStateCurr = GS_NONE;
	gGameStateNext = startState;
}

void GSM_Update()
{
	// If the state has changed
	if (gGameStateCurr != gGameStateNext || gGameStateNext == GS_RESTART)
	{
		// 1. Unload Current State
		if (GameStateFree) GameStateFree();
		if (GameStateUnload) GameStateUnload();

		if (gGameStateNext != GS_RESTART)
			gGameStateCurr = gGameStateNext;

		// 2. Load New State
		switch (gGameStateCurr)
		{
		case GS_MAINMENU:
			GameStateLoad = MainMenu_Load;
			GameStateInit = MainMenu_Initialize;
			GameStateUpdate = MainMenu_Update;
			GameStateDraw = MainMenu_Draw;
			GameStateFree = MainMenu_Free;
			GameStateUnload = MainMenu_Unload;
			break;

		case GS_Level1:
			GameStateLoad = Level1_Load;
			GameStateInit = Level1_Initialize;
			GameStateUpdate = Level1_Update;
			GameStateDraw = Level1_Draw;
			GameStateFree = Level1_Free;
			GameStateUnload = Level1_Unload;
			break;

		case GS_QUIT:
			// Handled in Main Loop
			break;

		default:
			break;
		}

		// 3. Initialize New State
		if (GameStateLoad) GameStateLoad();
		if (GameStateInit) GameStateInit();
	}

	// 4. Run Current State
	if (GameStateUpdate) GameStateUpdate();
	if (GameStateDraw) GameStateDraw();
}
