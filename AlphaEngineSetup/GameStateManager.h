#pragma once
#include "AEEngine.h"

// Defines the various operational states the application can reside in
typedef enum
{
    GS_RESTART,   // Command to re-initialize the current active state
    GS_QUIT,      // Command to terminate the main engine loop
    GS_MAINMENU,  // The interactive menu interface
    GS_LEVEL1 = 3, // for some reason this defaulted to int 8 and it messing with gs_level 6
    GS_LEVEL2,
    GS_LEVEL3,
    GS_LEVEL4,
    GS_LEVEL5,
    GS_LEVEL6,
    GS_NONE,
    GS_CREDIT,
    GS_LOGO,
} GAME_STATE;

// Global trackers for the state machine to determine when to switch logic blocks
extern GAME_STATE gGameStateCurr;
extern GAME_STATE gGameStateNext;
extern GAME_STATE gGameStatePrev;

// Prepares the manager to start at a specific state
void GSM_Initialize(GAME_STATE startState);

// Handles the logic for switching states and executing per-frame updates
void GSM_Update();

// Cleans up the current state before application exit
void GSM_Unload();