//#include "AudioSystem.h"
//#include <iostream>
//
//// --- Initialize Static Variables ---
//AEAudio AudioSystem::bgm = nullptr;
//AEAudio AudioSystem::sfxPickup = nullptr;
//AEAudio AudioSystem::sfxHurt = nullptr;
//AEAudio AudioSystem::sfxStun = nullptr;
//AEAudio AudioSystem::sfxDoorUnlock = nullptr;
//
//AEAudioGroup AudioSystem::groupMusic = nullptr;
//AEAudioGroup AudioSystem::groupSFX = nullptr;
//
//void AudioSystem::Load() {
//	// Create channels for volume control
//	groupMusic = AEAudioCreateGroup();
//	groupSFX = AEAudioCreateGroup();
//
//	// Set default volumes (Music at 50%, SFX at 80%)
//	AEAudioSetGroupVolume(groupMusic, 0.5f);
//	AEAudioSetGroupVolume(groupSFX, 0.8f);
//
//	// Load the files (Make sure these paths match your project!)
//	bgm = AEAudioLoadMusic("Assets/Audio/bgm.mp3");
//	sfxPickup = AEAudioLoadSound("Assets/Audio/pickup.wav");
//	sfxHurt = AEAudioLoadSound("Assets/Audio/hurt.wav");
//	sfxStun = AEAudioLoadSound("Assets/Audio/stun.wav");
//	sfxDoorUnlock = AEAudioLoadSound("Assets/Audio/unlock.wav");
//
//	std::cout << "Audio System Loaded!" << std::endl;
//}
//
//void AudioSystem::Unload() {
//	if (bgm) AEAudioUnloadAudio(bgm);
//	if (sfxPickup) AEAudioUnloadAudio(sfxPickup);
//	if (sfxHurt) AEAudioUnloadAudio(sfxHurt);
//	if (sfxStun) AEAudioUnloadAudio(sfxStun);
//	if (sfxDoorUnlock) AEAudioUnloadAudio(sfxDoorUnlock);
//}
//
//void AudioSystem::PlayBGM() {
//	if (bgm) {
//		// Play(audio, group, volume, pitch, loops)
//		// loops = -1 means it will loop forever!
//		AEAudioPlay(bgm, groupMusic, 1.0f, 1.0f, -1);
//	}
//}
//
//void AudioSystem::StopBGM() {
//	if (groupMusic) {
//		AEAudioStopGroup(groupMusic);
//	}
//}
//
//void AudioSystem::PlaySFX_Pickup() {
//	if (sfxPickup) AEAudioPlay(sfxPickup, groupSFX, 1.0f, 1.0f, 0);
//}
//
//void AudioSystem::PlaySFX_Hurt() {
//	if (sfxHurt) AEAudioPlay(sfxHurt, groupSFX, 1.0f, 1.0f, 0);
//}
//
//void AudioSystem::PlaySFX_Stun() {
//	if (sfxStun) AEAudioPlay(sfxStun, groupSFX, 1.0f, 1.0f, 0);
//}
//
//void AudioSystem::PlaySFX_DoorUnlock() {
//	if (sfxDoorUnlock) AEAudioPlay(sfxDoorUnlock, groupSFX, 1.0f, 1.0f, 0);
//}