// author : Tay Dylan
#include "AudioSystem.h"
#include <iostream>
#include <fstream>

// External linkage to volume and sync settings defined in the settings menu
extern float g_MusicVolume;
extern float g_SFXVolume;
extern bool g_VSyncEnabled;

// Global map to store sound instances by their string identifiers
std::map<std::string, AudioSystem::SoundInstance> AudioSystem::soundMap;

// Audio group handles for categorized volume control
AEAudioGroup AudioSystem::groupMusic = {};
AEAudioGroup AudioSystem::groupSFX = {};

void AudioSystem::Init()
{
    // Initialize audio groups via the alpha engine
    groupMusic = AEAudioCreateGroup();
    groupSFX = AEAudioCreateGroup();

    // Verify group validity before assigning default volume levels
    if (AEAudioIsValidGroup(groupMusic))
    {
        // Initialize background music to 50 percent
        AEAudioSetGroupVolume(groupMusic, 0.5f);
    }

    if (AEAudioIsValidGroup(groupSFX))
    {
        // Initialize sound effects to 80 percent
        AEAudioSetGroupVolume(groupSFX, 0.8f);
    }
}

void AudioSystem::LoadSound(const std::string& name, const std::string& path, bool isBGM)
{
    // Prevent redundant loading if the identifier already exists in the map
    if (soundMap.find(name) != soundMap.end()) return;

    SoundInstance si = {};
    si.isBGM = isBGM;

    // Utilize specific engine loaders based on the audio category
    if (isBGM)
    {
        si.audio = AEAudioLoadMusic(path.c_str());
    }
    else
    {
        si.audio = AEAudioLoadSound(path.c_str());
    }

    // Register the sound instance only if the handle was successfully created
    if (AEAudioIsValidAudio(si.audio))
    {
        soundMap[name] = si;
    }
}

void AudioSystem::Play(const std::string& name)
{
    // Default playback at unity gain
    Play(name, 1.0f);
}

void AudioSystem::Play(const std::string& name, float volumeMultiplier)
{
    // Search the resource map for the requested sound identifier
    auto it = soundMap.find(name);

    if (it != soundMap.end())
    {
        SoundInstance& si = it->second;

        // Apply local multipliers to the global volume settings
        float baseVolume = si.isBGM ? g_MusicVolume : g_SFXVolume;
        float finalVolume = baseVolume * volumeMultiplier;

        // Execute playback through the alpha engine with group-specific settings
        AEAudioPlay(si.audio, (si.isBGM ? groupMusic : groupSFX),
            finalVolume, 1.0f, (si.isBGM ? -1 : 0));
    }
}

void AudioSystem::PauseAll()
{
    // Suspend playback for both music and effect categories
    if (AEAudioIsValidGroup(groupMusic)) AEAudioPauseGroup(groupMusic);
    if (AEAudioIsValidGroup(groupSFX)) AEAudioPauseGroup(groupSFX);
}

void AudioSystem::ResumeAll()
{
    // Restore playback for both music and effect categories
    if (AEAudioIsValidGroup(groupMusic)) AEAudioResumeGroup(groupMusic);
    if (AEAudioIsValidGroup(groupSFX)) AEAudioResumeGroup(groupSFX);
}

void AudioSystem::StopGroup(bool bgm)
{
    // Terminate all active voices within the specified audio group
    if (bgm && AEAudioIsValidGroup(groupMusic))
    {
        AEAudioStopGroup(groupMusic);
    }
    else if (!bgm && AEAudioIsValidGroup(groupSFX))
    {
        AEAudioStopGroup(groupSFX);
    }
}

void AudioSystem::SetBGMVolume(float volume)
{
    // Update the master gain for the background music group
    if (AEAudioIsValidGroup(groupMusic))
    {
        AEAudioSetGroupVolume(groupMusic, volume);
    }
}

void AudioSystem::SetSFXVolume(float volume)
{
    // Update the master gain for the sound effects group
    if (AEAudioIsValidGroup(groupSFX))
    {
        AEAudioSetGroupVolume(groupSFX, volume);
    }
}

void AudioSystem::Exit()
{
    // Save current user settings to a local configuration file
    std::ofstream outFile("settings.txt");
    if (outFile.is_open())
    {
        outFile << g_MusicVolume << std::endl;
        outFile << g_SFXVolume << std::endl;
        outFile << g_VSyncEnabled << std::endl;
        outFile.close();
    }

    // Release all loaded audio resources to prevent memory leaks
    for (auto& pair : soundMap)
    {
        AEAudioUnloadAudio(pair.second.audio);
    }

    // Clear the resource map and release group handles
    soundMap.clear();

    if (AEAudioIsValidGroup(groupMusic))
    {
        AEAudioUnloadAudioGroup(groupMusic);
    }
    if (AEAudioIsValidGroup(groupSFX))
    {
        AEAudioUnloadAudioGroup(groupSFX);
    }
}