#include "audiomanager.h"
#include "raylib.h"

static Sound crashSound;
static Sound breakSound;
static Sound fuelSound;
static Sound nitroSound;
static Sound invisibleSound;
static Sound doubleHornSound;
static Sound policeSirenSound;
static Sound ambulanceSirenSound;
static Sound buttonHoverSound;
static Sound menuSelectSound;

static Music bgmMusic;
static Music menuMusic;

void InitAudioManager(void) {
    InitAudioDevice();
    
    crashSound = LoadSound("assets/crash.wav");
    breakSound = LoadSound("assets/break.mp3");
    fuelSound = LoadSound("assets/fuel.wav");
    nitroSound = LoadSound("assets/nitro.mp3");
    invisibleSound = LoadSound("assets/invisible.mp3");
    doubleHornSound = LoadSound("assets/double_horn.wav");
    policeSirenSound = LoadSound("assets/police_siren.mp3");
    ambulanceSirenSound = LoadSound("assets/ambulance_siren.mp3");
    
    buttonHoverSound = LoadSound("assets/hover.mp3");
    menuSelectSound = LoadSound("assets/select.mp3");

    bgmMusic = LoadMusicStream("assets/bgm1.mp3");
    SetMusicVolume(bgmMusic, 0.5f);

    menuMusic = LoadMusicStream("assets/bgm0.mp3");
    SetMusicVolume(menuMusic, 0.5f);
}

void CloseAudioManager(void) {
    UnloadSound(crashSound);
    UnloadSound(breakSound);
    UnloadSound(fuelSound);
    UnloadSound(nitroSound);
    UnloadSound(invisibleSound);
    UnloadSound(doubleHornSound);
    UnloadSound(policeSirenSound);
    UnloadSound(ambulanceSirenSound);
    UnloadSound(buttonHoverSound);
    UnloadSound(menuSelectSound);

    UnloadMusicStream(bgmMusic);
    UnloadMusicStream(menuMusic);

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

void PlayMenuBGM(void) {
    if (!IsMusicStreamPlaying(menuMusic)) {
        PlayMusicStream(menuMusic);
    }
}

void StopMenuBGM(void) {
    if (IsMusicStreamPlaying(menuMusic)) {
        StopMusicStream(menuMusic);
    }
}

void UpdateAudio(GameContext *game, float dt) {
    (void)dt;

    if (game->currentState == MENU) {
        if (IsMusicStreamPlaying(bgmMusic)) {
            StopMusicStream(bgmMusic);
        }
        if (!IsMusicStreamPlaying(menuMusic)) {
            PlayMusicStream(menuMusic);
        }
        UpdateMusicStream(menuMusic);
    } else if (game->currentState == GAME_OVER || game->currentState == PAUSED) {
        if (IsMusicStreamPlaying(bgmMusic)) {
            StopMusicStream(bgmMusic);
        }
        if (IsMusicStreamPlaying(menuMusic)) {
            StopMusicStream(menuMusic);
        }
    } else {
        if (IsMusicStreamPlaying(menuMusic)) {
            StopMusicStream(menuMusic);
        }
        if (!IsMusicStreamPlaying(bgmMusic)) {
            PlayMusicStream(bgmMusic);
        }
        UpdateMusicStream(bgmMusic);
    }
}

void PlayCrashSound(void) { PlaySound(crashSound); }
void PlayBreakSound(void) { PlaySound(breakSound); }
void PlayFuelSound(void) { PlaySound(fuelSound); }
void PlayNitroSound(void) { PlaySound(nitroSound); }
void PlayInvisibleSound(void) { PlaySound(invisibleSound); }
void PlayDoubleHornSound(void) { PlaySound(doubleHornSound); }
void PlayButtonHoverSound(void) { PlaySound(buttonHoverSound); }
void PlayMenuSelectSound(void) { PlaySound(menuSelectSound); }

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