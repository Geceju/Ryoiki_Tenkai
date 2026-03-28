//#pragma once
//
//#ifndef AUDIO_SYSTEM_H
//#define AUDIO_SYSTEM_H
//
//#include "AEEngine.h"
//
//class AudioSystem {
//public:
//	// Core system functions
//	static void Load();
//	static void Unload();
//
//	// Music controls
//	static void PlayBGM();
//	static void StopBGM();
//
//	// Sound Effect Triggers
//	static void PlaySFX_Pickup();
//	static void PlaySFX_Hurt();
//	static void PlaySFX_Stun();
//	static void PlaySFX_DoorUnlock();
//
//private:
//	// Audio file handles
//	static AEAudio bgm;
//	static AEAudio sfxPickup;
//	static AEAudio sfxHurt;
//	static AEAudio sfxStun;
//	static AEAudio sfxDoorUnlock;
//
//	// Audio groups (allows us to change music/sfx volume independently)
//	static AEAudioGroup groupMusic;
//	static AEAudioGroup groupSFX;
//};
//
//#endif