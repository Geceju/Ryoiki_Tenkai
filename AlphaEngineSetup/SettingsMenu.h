//author : Tay Dylan
#ifndef SETTINGS_MENU_H
#define SETTINGS_MENU_H

#include "AEEngine.h"

// Global settings shared across the entire game
extern float g_MusicVolume;
extern float g_SFXVolume;
extern bool g_VSyncEnabled;

// Core functions to manage the settings overlay
void SettingsMenu_Load();
void SettingsMenu_Initialize();
void SettingsMenu_Update(bool& isMenuOpen);
void SettingsMenu_Draw(bool isIngame); // Pass true if in a level to draw the "Exit to Menu" button
void SettingsMenu_Unload();

#endif