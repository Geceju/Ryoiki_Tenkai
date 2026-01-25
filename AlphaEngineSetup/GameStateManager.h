#pragma once
#include "AEEngine.h"

// Defines the various operational states the application can reside in
typedef enum
{
    GS_RESTART,   // Command to re-initialize the current active state
    GS_QUIT,      // Command to terminate the main engine loop
    GS_MAINMENU,  // The interactive menu interface
    GS_PLAY,      // The dungeon generation and gameplay environment
    GS_NONE       // A placeholder used during initial startup
} GAME_STATE;

// Global trackers for the state machine to determine when to switch logic blocks
extern GAME_STATE gGameStateCurr;
extern GAME_STATE gGameStateNext;
extern GAME_STATE gGameStatePrev;

// Prepares the manager to start at a specific state
void GSM_Initialize(GAME_STATE startState);

// Handles the logic for switching states and executing per-frame updates
void GSM_Update();