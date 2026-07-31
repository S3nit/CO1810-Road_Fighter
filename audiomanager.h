#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "game.h"

void InitAudioManager(void);
void CloseAudioManager(void);
void UpdateAudio(GameContext *game, float dt);

// Background Music controls
void PlayBGM(void);
void StopBGM(void);
void PlayMenuBGM(void);
void StopMenuBGM(void);

// Sound Effects
void PlayCrashSound(void);
void PlayBreakSound(void);
void PlayFuelSound(void);
void PlayNitroSound(void);
void PlayInvisibleSound(void);
void PlayDoubleHornSound(void);

// UI Sound Effects
void PlayButtonHoverSound(void);
void PlayMenuSelectSound(void);

void PlayPoliceSirenSound(void);
void FadePoliceSirenSound(void);
void StopPoliceSirenSound(void);
void PlayAmbulanceSirenSound(void);
void FadeAmbulanceSirenSound(void);
void StopAmbulanceSirenSound(void);
void SetAmbulanceSirenVolume(float volume);

#endif // AUDIOMANAGER_H