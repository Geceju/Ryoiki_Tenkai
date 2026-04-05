//author : Tay Dylan
#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "AEEngine.h"
#include <map>
#include <string>

class AudioSystem {
public:
    // Container for audio data and its classification
    struct SoundInstance {
        AEAudio audio;
        bool isBGM;
    };

    // Sets up the audio groups
    static void Init();
    // Adds a sound file to the library using the appropriate engine loader
    static void LoadSound(const std::string& name, const std::string& path, bool isBGM);
    // Triggers playback by nickname
    static void Play(const std::string& name);
    // Overloaded play (allows for a specific volume multiplier)
    static void Play(const std::string& name, float volumeMultiplier);
    // Suspends all active audio groups (Pause Menu)
    static void PauseAll();
    // Continues playback for all suspended audio groups
    static void ResumeAll();
    // Stops everything in a specific group
    static void StopGroup(bool bgm);
    // Global volume control for music
    static void SetBGMVolume(float volume);
    // Global volume control for effects
    static void SetSFXVolume(float volume);
    // Clears all library data and unloads groups
    static void Exit();

private:
    // Mapping of nicknames to sound data
    static std::map<std::string, SoundInstance> soundMap;
    // Handle for the music channel group
    static AEAudioGroup groupMusic;
    // Handle for the SFX channel group
    static AEAudioGroup groupSFX;
};

#endif