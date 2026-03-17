#include "AEEngine.h"
#include "GameStateManager.h"
#include "Level.h"
#include "MainMenu.h"
#include "AEInput.h"

// Tracking variables for the current and next game states
GAME_STATE gGameStateCurr = GS_NONE;
GAME_STATE gGameStateNext = GS_NONE;
GAME_STATE gGameStatePrev = GS_NONE;

// Function pointers used to execute the logic of whichever state is active
void (*GameStateLoad)() = nullptr;
void (*GameStateInit)() = nullptr;
void (*GameStateUpdate)() = nullptr;
void (*GameStateDraw)() = nullptr;
void (*GameStateFree)() = nullptr;
void (*GameStateUnload)() = nullptr;

void GSM_Initialize(GAME_STATE startState)
{
    // Start the manager with no active state and set the target state
    gGameStateCurr = GS_NONE;
    gGameStateNext = startState;
}

void GSM_Update()
{
	// If the state has changed
	if (gGameStateCurr != gGameStateNext || gGameStateNext == GS_RESTART)
	{
		// Unload Current State
		if (GameStateFree) GameStateFree();
		if (GameStateUnload) GameStateUnload();

		if (gGameStateNext != GS_RESTART)
        {    
            gGameStatePrev = gGameStateCurr;
			gGameStateCurr = gGameStateNext;
        }
        else
        {
            gGameStateNext = gGameStateCurr;
        }

		// CLEAR POINTERS HERE: Prevents the phantom frame bug
		GameStateLoad = nullptr;
		GameStateInit = nullptr;
		GameStateUpdate = nullptr;
		GameStateDraw = nullptr;
		GameStateFree = nullptr;
		GameStateUnload = nullptr;

		// Load New State
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

		case GS_LEVEL1:

		case GS_LEVEL2:

		case GS_LEVEL3:

		case GS_LEVEL4:

		case GS_LEVEL5:

		case GS_LEVEL6:

			GameStateLoad = Level_Load;
			GameStateInit = Level_Init;
			GameStateUpdate = Level_Update;
			GameStateDraw = Level_Draw;
			GameStateFree = Level_Free;
			GameStateUnload = Level_Unload;
			break;

		case GS_QUIT:
			break;

		default:
			break;
		}

		// Initialize new state
		if (GameStateLoad) GameStateLoad();
		if (GameStateInit) GameStateInit();
	}

	// Run current state
	if (GameStateUpdate) GameStateUpdate();
	if (GameStateDraw) GameStateDraw();
}

void GSM_Unload()
{
	// Instead of a switch, use the pointers we already set up!
		// If we are in MainMenu, these point to MainMenu_Free/Unload automatically.
	if (GameStateFree) GameStateFree();
	if (GameStateUnload) GameStateUnload();

	// Reset pointers to be safe
	GameStateLoad = nullptr;
	GameStateInit = nullptr;
	GameStateUpdate = nullptr;
	GameStateDraw = nullptr;
	GameStateFree = nullptr;
	GameStateUnload = nullptr;
}
