#include "AssetLoader.h"
#include "AudioSystem.h"
#include <fstream>

// External references to your volume variables from SettingsMenu.cpp
extern float g_MusicVolume;
extern float g_SFXVolume;
extern bool g_VSyncEnabled;

void LoadAllAssets() {
	// Initialize the audio groups first
	AudioSystem::Init();

	// Attempt to load saved settings
	std::ifstream inFile("settings.txt");
	if (inFile.is_open()) {
		inFile >> g_MusicVolume;
		inFile >> g_SFXVolume;
		inFile >> g_VSyncEnabled;
		inFile.close();

		// Apply the loaded volumes immediately
		AudioSystem::SetBGMVolume(g_MusicVolume);
		AudioSystem::SetSFXVolume(g_SFXVolume);
	}

	// Background Music
	AudioSystem::LoadSound( "MenuBGM", "Assets/Sounds/ambience.wav", true);
	AudioSystem::LoadSound("LevelBGM", "Assets/Sounds/ambience.wav", true);

	// SFX
	AudioSystem::LoadSound("Click", "Assets/Sounds/button_click.wav", false);	// Played when the player clicks a button in the menu or settings

	// Abilities
	AudioSystem::LoadSound("Speed",    "Assets/Sounds/speedup.wav", false);		// Played when the player uses the speed boost ability
	AudioSystem::LoadSound( "Stun",	      "Assets/Sounds/stun.wav", false);		// Played when the player uses the stun ability
	AudioSystem::LoadSound( "Path", "Assets/Sounds/pathfinder.wav", false);		// Played when the player uses the pathfinding ability

	// Player interactions
	AudioSystem::LoadSound("Footsteps",   "Assets/Sounds/footsteps.wav", false);	// Looping sound played when the player is moving, stops when they are still
	AudioSystem::LoadSound(      "Hit",		   "Assets/Sounds/ouch.wav", false);	// Played when the player takes damage
	AudioSystem::LoadSound(   "Pickup", "Assets/Sounds/item_pickup.wav", false);	// Played when the player collects a key or other item
	AudioSystem::LoadSound(  "AllKeys",    "Assets/Sounds/all_keys.wav", false);	// Played when the player collects all keys in a level
	AudioSystem::LoadSound("Heartbeat",   "Assets/Sounds/heartbeat.wav", false);	// Played when the enemy is close to the player, with volume increasing as they get closer

	// Can also add texture loading here in the future
}