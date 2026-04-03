#include "AudioSystem.h"
#include <iostream>
#include <fstream>

// Link to global volume variables defined in SettingsMenu.cpp
extern float g_MusicVolume;
extern float g_SFXVolume;
extern bool g_VSyncEnabled;

// Initialize the sound library map
std::map<std::string, AudioSystem::SoundInstance> AudioSystem::soundMap;
// Initialize the Music group
AEAudioGroup AudioSystem::groupMusic = {};
// Initialize the SFX group
AEAudioGroup AudioSystem::groupSFX = {};

void AudioSystem::Init() {
	// Create the background music group through the Alpha Engine
	groupMusic = AEAudioCreateGroup();
	// Create the sound effects group through the Alpha Engine
	groupSFX = AEAudioCreateGroup();

	// Use the engine's validity check before setting volume
	if (AEAudioIsValidGroup(groupMusic)) {
		// Sets background music to 50% volume
		AEAudioSetGroupVolume(groupMusic, 0.5f);
	}

	if (AEAudioIsValidGroup(groupSFX)) {
		// Sets sound effects to 80% volume
		AEAudioSetGroupVolume(groupSFX, 0.8f);
	}
}

void AudioSystem::LoadSound(const std::string& name, const std::string& path, bool isBGM) {
	// Prevent loading if the nickname already exists in the map
	if (soundMap.find(name) != soundMap.end()) return;

	SoundInstance si = {};
	si.isBGM = isBGM;

	// Use specific engine loaders based on sound type
	if (isBGM) {
		si.audio = AEAudioLoadMusic(path.c_str());
	}
	else {
		si.audio = AEAudioLoadSound(path.c_str());
	}

	// Only add to map if the audio handle is valid
	if (AEAudioIsValidAudio(si.audio)) {
		soundMap[name] = si;
	}
}

void AudioSystem::Play(const std::string& name) {
	// Search the map for the nickname
	auto it = soundMap.find(name);

	if (it != soundMap.end()) {
		SoundInstance& si = it->second;

		if (si.isBGM) {
			// Play on music group with loop forever (-1)
			AEAudioPlay(si.audio, groupMusic, 1.0f, 1.0f, -1);
		}
		else {
			// Play on SFX group to play once (0)
			AEAudioPlay(si.audio, groupSFX, 1.0f, 1.0f, 0);
		}
	}
}

void AudioSystem::PauseAll() {
	// Pause both groups if they are valid
	if (AEAudioIsValidGroup(groupMusic)) AEAudioPauseGroup(groupMusic);
	if (AEAudioIsValidGroup(groupSFX)) AEAudioPauseGroup(groupSFX);
}

void AudioSystem::ResumeAll() {
	// Resume both groups if they are valid
	if (AEAudioIsValidGroup(groupMusic)) AEAudioResumeGroup(groupMusic);
	if (AEAudioIsValidGroup(groupSFX)) AEAudioResumeGroup(groupSFX);
}

void AudioSystem::StopGroup(bool bgm) {
	// Stops either the music or SFX group entirely
	if (bgm && AEAudioIsValidGroup(groupMusic)) {
		AEAudioStopGroup(groupMusic);
	}
	else if (!bgm && AEAudioIsValidGroup(groupSFX)) {
		AEAudioStopGroup(groupSFX);
	}
}

void AudioSystem::SetBGMVolume(float volume) {
	// Safely update volume if group is valid
	if (AEAudioIsValidGroup(groupMusic)) {
		AEAudioSetGroupVolume(groupMusic, volume);
	}
}

void AudioSystem::SetSFXVolume(float volume) {
	// Safely update volume if group is valid
	if (AEAudioIsValidGroup(groupSFX)) {
		AEAudioSetGroupVolume(groupSFX, volume);
	}
}

void AudioSystem::Exit() {
	// Save current volume levels to a file
	std::ofstream outFile("settings.txt");
	if (outFile.is_open()) {
		outFile << g_MusicVolume << std::endl;
		outFile << g_SFXVolume << std::endl;
		outFile << g_VSyncEnabled << std::endl;
		outFile.close();
	}

	// Unload each audio handle
	for (auto& pair : soundMap) {
		// Use the specific function from AEAudio.h
		AEAudioUnloadAudio(pair.second.audio);
	}

	// This removes all elements from the map
	soundMap.clear();

	// Safety check: delete the groups if they were created
	if (AEAudioIsValidGroup(groupMusic)) {
		AEAudioUnloadAudioGroup(groupMusic);
	}
	if (AEAudioIsValidGroup(groupSFX)) {
		AEAudioUnloadAudioGroup(groupSFX);
	}

	// Optional: Log to console to verify this ran
	// std::cout << "Audio System cleaned" << std::endl;
}