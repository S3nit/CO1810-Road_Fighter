#include "audiomanager.h"
#include "raylib.h"

static Sound crashSound;
static Sound breakSound;
static Sound fuelSound;
static Sound doubleHornSound;
static Sound policeSirenSound;
static Sound ambulanceSirenSound;

static Music bgmMusic;

void InitAudioManager(void) {
    InitAudioDevice();
    
    crashSound = LoadSound("assets/crash.wav");
    breakSound = LoadSound("assets/break.mp3");
    fuelSound = LoadSound("assets/fuel.wav");
    doubleHornSound = LoadSound("assets/double_horn.wav");
    policeSirenSound = LoadSound("assets/police_siren.mp3");
    ambulanceSirenSound = LoadSound("assets/ambulance_siren.mp3");

    bgmMusic = LoadMusicStream("assets/bgm1.mp3");
    SetMusicVolume(bgmMusic, 0.5f);
}

void CloseAudioManager(void) {
    UnloadSound(crashSound);
    UnloadSound(breakSound);
    UnloadSound(fuelSound);
    UnloadSound(doubleHornSound);
    UnloadSound(policeSirenSound);
    UnloadSound(ambulanceSirenSound);

    UnloadMusicStream(bgmMusic);

    CloseAudioDevice();
}

void PlayBGM(void) {
    if (!IsMusicStreamPlaying(bgmMusic)) {
        PlayMusicStream(bgmMusic);
    }
}

void StopBGM(void) {
    if (IsMusicStreamPlaying(bgmMusic)) {
        StopMusicStream(bgmMusic);
    }
}

void UpdateAudio(GameContext *game, float dt) {
    (void)dt;

    if (game->currentState == GAME_OVER || game->currentState == MENU) {
        if (IsMusicStreamPlaying(bgmMusic)) {
            StopMusicStream(bgmMusic);
        }
    } else {
        if (!IsMusicStreamPlaying(bgmMusic)) {
            PlayMusicStream(bgmMusic);
        }
        UpdateMusicStream(bgmMusic);
    }
}

void PlayCrashSound(void) { PlaySound(crashSound); }
void PlayBreakSound(void) { PlaySound(breakSound); }
void PlayFuelSound(void) { PlaySound(fuelSound); }
void PlayDoubleHornSound(void) { PlaySound(doubleHornSound); }

void PlayPoliceSirenSound(void) {
    if (!IsSoundPlaying(policeSirenSound)) PlaySound(policeSirenSound);
}

void FadePoliceSirenSound(void) {
    SetSoundVolume(policeSirenSound, 0.3f);
}

void StopPoliceSirenSound(void) {
    StopSound(policeSirenSound);
    SetSoundVolume(policeSirenSound, 1.0f);
}

void PlayAmbulanceSirenSound(void) {
    if (!IsSoundPlaying(ambulanceSirenSound)) PlaySound(ambulanceSirenSound);
}

void FadeAmbulanceSirenSound(void) {
    SetSoundVolume(ambulanceSirenSound, 0.3f);
}

void StopAmbulanceSirenSound(void) {
    StopSound(ambulanceSirenSound);
    SetSoundVolume(ambulanceSirenSound, 1.0f);
}

void SetAmbulanceSirenVolume(float volume) {
    if (volume < 0.0f) volume = 0.0f;
    if (volume > 1.0f) volume = 1.0f;
    SetSoundVolume(ambulanceSirenSound, volume);
}