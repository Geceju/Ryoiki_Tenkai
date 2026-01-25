#pragma once
#include "AEEngine.h" // Standard Alpha Engine Include

// 1. Define the list of all possible states
typedef enum
{
    GS_RESTART, // Special state to restart current level
    GS_QUIT,    // Special state to close application
    GS_MAINMENU,
    GS_PLAY,    // The actual game
    GS_NONE     // Initialization value
} GAME_STATE;

// 2. Global variables (accessed by Main.c and levels)
extern GAME_STATE gGameStateCurr;
extern GAME_STATE gGameStateNext;
extern GAME_STATE gGameStatePrev;

// 3. Function Prototypes for the State Manager
void GSM_Initialize(GAME_STATE startState);
void GSM_Update();