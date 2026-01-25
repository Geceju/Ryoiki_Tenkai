#include "AEEngine.h"
#include "GameStateManager.h"
#include "Level.h"
#include "MainMenu.h"

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
    // Check if the state needs to change or if the current state needs to restart
    if (gGameStateCurr != gGameStateNext || gGameStateNext == GS_RESTART)
    {
        // Clean up the memory and assets of the state we are currently leaving
        if (GameStateFree)
        {
            GameStateFree();
        }

        if (GameStateUnload)
        {
            GameStateUnload();
        }

        // Set the current state to the next state, unless we are restarting
        if (gGameStateNext != GS_RESTART)
        {
            gGameStatePrev = gGameStateCurr;
            gGameStateCurr = gGameStateNext;
        }
        else
        {
            gGameStateNext = gGameStateCurr;
        }

        // Map the function pointers to the specific logic of the selected state
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

        case GS_PLAY:
            GameStateLoad = Level_Load;
            GameStateInit = Level_Init;
            GameStateUpdate = Level_Update;
            GameStateDraw = Level_Draw;
            GameStateFree = Level_Free;
            GameStateUnload = Level_Unload;
            break;

        case GS_QUIT:
            // The main loop in Main.cpp will check for GS_QUIT to close the window
            break;

        default:
            break;
        }

        // Load the assets and run the initialization for the new state
        if (GameStateLoad)
        {
            GameStateLoad();
        }

        if (GameStateInit)
        {
            GameStateInit();
        }
    }

    // Execute the logic and rendering for the state that is currently running
    if (GameStateUpdate)
    {
        GameStateUpdate();
    }

    if (GameStateDraw)
    {
        GameStateDraw();
    }
}

//#include "AEEngine.h"
//#include "GameStateManager.h"
//
//// Include Level Headers
//// (You would also include Level_Play.h here)
//void MainMenu_Load();
//void MainMenu_Initialize();
//void MainMenu_Update();
//void MainMenu_Draw();
//void MainMenu_Free();
//void MainMenu_Unload();
//
//// Global State Variables
//GAME_STATE gGameStateCurr = GS_NONE;
//GAME_STATE gGameStateNext = GS_NONE;
//GAME_STATE gGameStatePrev = GS_NONE;
//
//// Function Pointers
//void (*GameStateLoad)() = 0;
//void (*GameStateInit)() = 0;
//void (*GameStateUpdate)() = 0;
//void (*GameStateDraw)() = 0;
//void (*GameStateFree)() = 0;
//void (*GameStateUnload)() = 0;
//
//void GSM_Initialize(GAME_STATE startState)
//{
//    gGameStateCurr = GS_NONE;
//    gGameStateNext = startState;
//}
//
//void GSM_Update()
//{
//    // If the state has changed
//    if (gGameStateCurr != gGameStateNext || gGameStateNext == GS_RESTART)
//    {
//        // 1. Unload Current State
//        if (GameStateFree) GameStateFree();
//        if (GameStateUnload) GameStateUnload();
//
//        if (gGameStateNext != GS_RESTART)
//            gGameStateCurr = gGameStateNext;
//
//        // 2. Load New State
//        switch (gGameStateCurr)
//        {
//        case GS_MAINMENU:
//            GameStateLoad = MainMenu_Load;
//            GameStateInit = MainMenu_Initialize;
//            GameStateUpdate = MainMenu_Update;
//            GameStateDraw = MainMenu_Draw;
//            GameStateFree = MainMenu_Free;
//            GameStateUnload = MainMenu_Unload;
//            break;
//
//        case GS_PLAY:
//            // Point to your Game Level functions here
//            // GameStateLoad = Game_Load; ... etc
//            break;
//
//        case GS_QUIT:
//            // Handled in Main Loop
//            break;
//
//        default:
//            break;
//        }
//
//        // 3. Initialize New State
//        if (GameStateLoad) GameStateLoad();
//        if (GameStateInit) GameStateInit();
//    }
//
//    // 4. Run Current State
//    if (GameStateUpdate) GameStateUpdate();
//    if (GameStateDraw) GameStateDraw();
//}
