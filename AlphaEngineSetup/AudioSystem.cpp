#include "AudioSystem.h"
#include <iostream>

// Initialize the sound library map
std::map<std::string, AudioSystem::SoundInstance> AudioSystem::soundMap;
// Initialize the Music group struct with {} to zero out internal pointers
AEAudioGroup AudioSystem::groupMusic = {};
// Initialize the SFX group struct with {} to zero out internal pointers
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

    // Use the engine's validity check before setting volume
    if (AEAudioIsValidGroup(groupSFX)) {
        // Sets sound effects to 80% volume
        AEAudioSetGroupVolume(groupSFX, 0.8f);
    }

    std::cout << "Audio System: Channels Initialized" << std::endl;
}

void AudioSystem::LoadSound(const std::string& name, const std::string& path, bool isBGM) {
    // Initialize the audio struct to a null state
    AEAudio newAudio = {};

    if (isBGM) {
        // Load long files as Music (streaming)
        newAudio = AEAudioLoadMusic(path.c_str());
    }
    else {
        // Load short files as Sound (RAM-resident)
        newAudio = AEAudioLoadSound(path.c_str());
    }

    // Check the internal FMOD pointer to verify the file loaded successfully
    if (newAudio.fmod_sound != nullptr) {
        // Create a temporary storage instance
        SoundInstance si;
        // Assign the engine's audio struct
        si.audio = newAudio;
        // Mark whether this should loop (BGM) or play once (SFX)
        si.isBGM = isBGM;
        // Store the instance in our map using the nickname
        soundMap[name] = si;
    }
    else {
        // Error logging if the file is missing or invalid
        std::cerr << "Audio System Error: Failed to load " << path << std::endl;
    }
}

void AudioSystem::Play(const std::string& name) {
    // Search the library map for the nickname
    auto it = soundMap.find(name);

    // Check if the name exists in the library
    if (it != soundMap.end()) {
        // Get a reference to the sound data in the map
        SoundInstance& si = it->second;

        if (si.isBGM) {
            // Play using music group with -1 (infinite loop)
            AEAudioPlay(si.audio, groupMusic, 1.0f, 1.0f, -1);
        }
        else {
            // Play using SFX group with 0 (play once)
            AEAudioPlay(si.audio, groupSFX, 1.0f, 1.0f, 0);
        }
    }
}

void AudioSystem::SetBGMVolume(float volume) {
    // Safely update volume if the group handle is valid
    if (AEAudioIsValidGroup(groupMusic)) {
        AEAudioSetGroupVolume(groupMusic, volume);
    }
}

void AudioSystem::SetSFXVolume(float volume) {
    // Safely update volume if the group handle is valid
    if (AEAudioIsValidGroup(groupSFX)) {
        AEAudioSetGroupVolume(groupSFX, volume);
    }
}

void AudioSystem::Exit() {
    // Loop through every sound currently in the library
    for (auto it = soundMap.begin(); it != soundMap.end(); ++it) {
        // Check if the sound struct has a valid internal pointer
        if (it->second.audio.fmod_sound != nullptr) {
            // Free the memory used by this audio file
            AEAudioUnloadAudio(it->second.audio);
        }
    }
    // Clear the map entries
    soundMap.clear();
    std::cout << "Audio System: Memory Cleaned" << std::endl;
}