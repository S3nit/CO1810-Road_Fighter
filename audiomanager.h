#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include "game.h"

void InitAudioManager(void);
void CloseAudioManager(void);
void UpdateAudio(GameContext *game, float dt);

// Background Music controls
void PlayBGM(void);
void StopBGM(void);

// Sound Effects
void PlayCrashSound(void);
void PlayBreakSound(void);
void PlayFuelSound(void);
void PlayDoubleHornSound(void);

void PlayPoliceSirenSound(void);
void FadePoliceSirenSound(void);
void StopPoliceSirenSound(void);

void PlayAmbulanceSirenSound(void);
void FadeAmbulanceSirenSound(void);
void StopAmbulanceSirenSound(void);
void SetAmbulanceSirenVolume(float volume);

#endif // AUDIOMANAGER_H