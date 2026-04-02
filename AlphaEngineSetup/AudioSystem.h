#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "AEEngine.h"
#include <string>
#include <map>

class AudioSystem {
public:
    static void Init();

	// ***************** LOADSOUND: MAIN FUNCTION WE USE TO LOAD AUDIO FILES *****************
	// *  name: a unique identifier for the sound (e.g., "explosion", "background_music") set
    //          by YOU for future reference when playing the sound
	// *  path: the file path to the audio asset (relative to the executable or project root)
	// * isBGM: set to true if this is background music that should loop, leave false for SFX
    // *
    // * Example to use function: AudioSystem::LoadSound("nickname", "path/to/file.wav", false);
    static void LoadSound(const std::string& name, const std::string& path, bool isBGM);
    // ***************************************************************************************

	// ********************************** PLAY **********************************
    // * Plays a sound by its given nickname (WHICH YOU SET FROM LOADSOUND)
    // * If it's marked as BGM, it will loop, otherwise it will play one cycle
    static void Play(const std::string& name);
    // **************************************************************************

	// Frees all loaded audio resources and clears the sound map; should be called on program exit (QUIT button or window close)
    static void Exit();

	// Self-explanatory volume control functions for BGM and SFX groups; volume should be between 0.0f (silent) and 1.0f (full volume)
    static void SetBGMVolume(float volume);
    static void SetSFXVolume(float volume);

private:
    struct SoundInstance {
        AEAudio audio;
        bool isBGM;
    };

    static std::map<std::string, SoundInstance> soundMap;
    static AEAudioGroup groupMusic;
    static AEAudioGroup groupSFX;
};

#endif