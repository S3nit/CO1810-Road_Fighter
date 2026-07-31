#include "game.h"
#include "audiomanager.h"
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdio.h>

const int SCREEN_WIDTH = 400;
const int SCREEN_HEIGHT = 600;
const float LANE_CENTERS[3] = { 100.0f, 200.0f, 300.0f };

static float SmoothLerp(float start, float end, float amount) {
    return start + amount * (end - start);
}

static float ClampFloat(float val, float min, float max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

void LoadUserDataFromCSV(GameContext *game) {
    FILE *file = fopen("scores.csv", "r");
    if (!file) {
        game->highScore = 0;
        game->attempts = 0;
        return;
    }

    char line[256];
    bool found = false;
    while (fgets(line, sizeof(line), file)) {
        char name[32];
        int hs = 0;
        int att = 0;

        char *token = strtok(line, ",");
        if (token) {
            strcpy(name, token);
            name[strcspn(name, "\r\n")] = 0;

            token = strtok(NULL, ",");
            if (token) hs = atoi(token);

            token = strtok(NULL, ",");
            if (token) att = atoi(token);

            if (strcmp(name, game->playerName) == 0) {
                game->highScore = hs;
                game->attempts = att;
                found = true;
                break;
            }
        }
    }
    fclose(file);

    if (!found) {
        game->highScore = 0;
        game->attempts = 0;
    }
}

void SaveUserDataToCSV(const GameContext *game) {
    typedef struct {
        char name[32];
        int highScore;
        int attempts;
    } UserRecord;

    UserRecord records[100];
    int recordCount = 0;
    bool found = false;

    FILE *file = fopen("scores.csv", "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file) && recordCount < 100) {
            char name[32];
            int hs = 0;
            int att = 0;

            char lineCopy[256];
            strcpy(lineCopy, line);

            char *token = strtok(lineCopy, ",");
            if (token) {
                strcpy(name, token);
                name[strcspn(name, "\r\n")] = 0;

                token = strtok(NULL, ",");
                if (token) hs = atoi(token);

                token = strtok(NULL, ",");
                if (token) att = atoi(token);

                if (strcmp(name, game->playerName) == 0) {
                    if (game->highScore > hs) {
                        hs = game->highScore;
                    }
                    att = game->attempts;
                    found = true;
                }

                strcpy(records[recordCount].name, name);
                records[recordCount].highScore = hs;
                records[recordCount].attempts = att;
                recordCount++;
            }
        }
        fclose(file);
    }

    if (!found && recordCount < 100) {
        strcpy(records[recordCount].name, game->playerName);
        records[recordCount].highScore = game->highScore;
        records[recordCount].attempts = game->attempts;
        recordCount++;
    }

    file = fopen("scores.csv", "w");
    if (file) {
        for (int i = 0; i < recordCount; i++) {
            fprintf(file, "%s,%d,%d\n", records[i].name, records[i].highScore, records[i].attempts);
        }
        fclose(file);
    }
}

static float GetLaneClearanceAhead(const GameContext *game, int lane, float ambY) {
    float nearestObstacleDist = 999.0f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active && (game->enemies[i].targetLane == lane || game->enemies[i].currentLane == lane)) {
            float distY = ambY - game->enemies[i].rect.y;
            if (distY > -40.0f && distY < nearestObstacleDist) {
                nearestObstacleDist = distY;
            }
        }
    }

    float playerXCenter = game->player.rect.x + (game->player.rect.width / 2.0f);
    float laneXCenter = LANE_CENTERS[lane];
    if (fabsf(playerXCenter - laneXCenter) < 35.0f) {
        float distYPlayer = ambY - game->player.rect.y;
        if (distYPlayer > -40.0f && distYPlayer < nearestObstacleDist) {
            nearestObstacleDist = distYPlayer;
        }
    }

    return nearestObstacleDist;
}

void InitGame(GameContext *game) {
    char tempName[32];
    if (game->playerName[0] != '\0') {
        strcpy(tempName, game->playerName);
    } else {
        strcpy(tempName, "Player");
    }
    int savedHighScore = game->highScore;
    int savedAttempts = game->attempts;

    game->currentState = MENU;
    game->player = (Player){
        .rect = { SCREEN_WIDTH / 2.0f - 15.0f, SCREEN_HEIGHT - 120.0f, 30.0f, 50.0f },
        .velocityX = 0.0f,
        .color = (Color){ 220, 50, 70, 255 },
        .skidTimer = 0.0f,
        .skidDirection = 0
    };

    for (int i = 0; i < MAX_ENEMIES; i++) {
        game->enemies[i].active = false;
    }

    for (int i = 0; i < MAX_FUELS; i++) {
        game->fuels[i].active = false;
    }

    for (int i = 0; i < MAX_NITROS; i++) {
        game->nitros[i].active = false;
    }

    for (int i = 0; i < MAX_INVISIBLES; i++) {
        game->invisibles[i].active = false;
    }

    game->police = (PoliceCar){
        .rect = { SCREEN_WIDTH / 2.0f - 15.0f, SCREEN_HEIGHT + 60.0f, 30.0f, 50.0f },
        .active = false,
        .hitCount = 0,
        .targetScoreThreshold = 10000.0f,
        .speed = 70.0f,
        .startScore = 0.0f
    };

    game->ambulance = (Ambulance){
        .rect = { 0, SCREEN_HEIGHT + 220.0f, 32.0f, 58.0f },
        .active = false,
        .speed = 0.0f,
        .currentLane = 1,
        .targetLane = 1,
        .isShifting = false,
        .shiftCooldown = 0.0f
    };

    game->roadOffset = 0.0f;
    game->gameSpeed = 200.0f;
    game->floatScore = 0.0f;
    game->fuel = 100.0f;
    game->nitroTimer = 0.0f;
    game->invisibleTimer = 0.0f;

    strcpy(game->playerName, tempName);
    game->highScore = savedHighScore;
    game->attempts = savedAttempts;
    game->editingName = false;
}

void UpdateGame(GameContext *game, float dt) {
    if (dt > 0.05f) dt = 0.05f;

    if (game->currentState == MENU) {
        return;
    }

    if (game->currentState == PLAYING || game->currentState == SKIDDING) {
        
        if (IsKeyPressed(KEY_ESCAPE)) {
            game->currentState = PAUSED;
            return;
        }

        if (game->nitroTimer > 0.0f) {
            game->nitroTimer -= dt;
        }
        if (game->invisibleTimer > 0.0f) {
            game->invisibleTimer -= dt;
        }

        game->roadOffset += game->gameSpeed * dt;
        if (game->roadOffset >= 40.0f) game->roadOffset = 0.0f;

        if (game->fuel <= 0) {
            game->fuel = 0;
            if ((int)game->floatScore > game->highScore) {
                game->highScore = (int)game->floatScore;
            }
            game->currentState = GAME_OVER;
            SaveUserDataToCSV(game);
            if (game->police.active) {
                game->police.active = false;
                FadePoliceSirenSound();
            }
            if (game->ambulance.active) {
                game->ambulance.active = false;
                StopAmbulanceSirenSound();
            }
        }

        if (game->currentState == PLAYING) {
            bool isAccelerating = IsKeyDown(KEY_UP);
            bool isBraking = IsKeyDown(KEY_DOWN);

            float maxAllowedSpeed = 650.0f;
            if (game->nitroTimer > 0.0f) {
                maxAllowedSpeed = 850.0f;
            }

            if (isAccelerating) {
                float accelRate = (game->nitroTimer > 0.0f) ? 800.0f : 400.0f;
                game->gameSpeed += accelRate * dt;
            } else if (isBraking) {
                game->gameSpeed -= 500.0f * dt;
            } else {
                if (game->gameSpeed > 200.0f) game->gameSpeed -= 180.0f * dt;
                else if (game->gameSpeed < 200.0f) game->gameSpeed += 120.0f * dt;
            }

            if (game->gameSpeed > maxAllowedSpeed) game->gameSpeed = maxAllowedSpeed;
            if (game->gameSpeed < 50.0f) game->gameSpeed = 50.0f;

            float targetVelocityX = 0.0f;
            if (IsKeyDown(KEY_LEFT))  targetVelocityX = -320.0f;
            if (IsKeyDown(KEY_RIGHT)) targetVelocityX = 320.0f;

            float steeringResponse = (targetVelocityX == 0.0f) ? 12.0f : 16.0f;
            game->player.velocityX = SmoothLerp(game->player.velocityX, targetVelocityX, steeringResponse * dt);
            game->player.rect.x += game->player.velocityX * dt;

            if (game->player.rect.x <= 50) {
                game->player.rect.x = 50;
                game->player.velocityX = 0.0f;
            } else if (game->player.rect.x >= SCREEN_WIDTH - 50 - game->player.rect.width) {
                game->player.rect.x = SCREEN_WIDTH - 50 - game->player.rect.width;
                game->player.velocityX = 0.0f;
            }

            float drainRate = 0.4f;
            if (isAccelerating) drainRate += 0.8f;
            drainRate += (game->gameSpeed * game->gameSpeed) * 0.000005f;
            game->fuel -= drainRate * dt;

            game->floatScore += (game->gameSpeed / 100.0f) * 15.0f * dt;
            if ((int)game->floatScore > game->highScore) {
                game->highScore = (int)game->floatScore;
            }
        }

        if (game->currentState == SKIDDING) {
            game->gameSpeed = SmoothLerp(game->gameSpeed, 100.0f, 3.0f * dt);
            game->fuel -= 0.4f * dt;
            game->player.skidTimer -= dt;

            game->player.velocityX = game->player.skidDirection * 280.0f;
            game->player.rect.x += game->player.velocityX * dt;

            if ((game->player.skidDirection == -1 && IsKeyPressed(KEY_RIGHT)) ||
                (game->player.skidDirection == 1 && IsKeyPressed(KEY_LEFT))) {
                game->currentState = PLAYING;
                game->player.velocityX = 0.0f;
            }

            if (game->player.skidTimer <= 0.0f) game->currentState = PLAYING;

            if (game->player.rect.x <= 50 || game->player.rect.x >= SCREEN_WIDTH - 50 - game->player.rect.width) {
                PlayCrashSound();
                if ((int)game->floatScore > game->highScore) {
                    game->highScore = (int)game->floatScore;
                }
                game->currentState = GAME_OVER;
                SaveUserDataToCSV(game);
                if (game->police.active) {
                    game->police.active = false;
                    FadePoliceSirenSound();
                }
                if (game->ambulance.active) {
                    game->ambulance.active = false;
                    StopAmbulanceSirenSound();
                }
            }
        }

        // --- FUEL ITEMS ---
        for (int i = 0; i < MAX_FUELS; i++) {
            if (!game->fuels[i].active) {
                if (GetRandomValue(0, 300) < 1) {
                    int lane = GetRandomValue(0, 2);
                    game->fuels[i].active = true;
                    game->fuels[i].lane = lane;
                    game->fuels[i].rect = (Rectangle){ LANE_CENTERS[lane] - 12.0f, -50.0f, 24.0f, 30.0f };
                    game->fuels[i].speed = (float)GetRandomValue(80, 140);
                    game->fuels[i].color = (Color){ 240, 80, 160, 255 };
                }
            } else {
                game->fuels[i].rect.y += (game->gameSpeed - game->fuels[i].speed) * dt;
                if (game->fuels[i].rect.y > SCREEN_HEIGHT) game->fuels[i].active = false;

                if (CheckCollisionRecs(game->player.rect, game->fuels[i].rect)) {
                    game->fuels[i].active = false;
                    PlayFuelSound();
                    game->fuel += 30.0f;
                    if (game->fuel > 100.0f) game->fuel = 100.0f;
                }
            }
        }

        // --- NITRO ITEMS (BLUE) ---
        for (int i = 0; i < MAX_NITROS; i++) {
            if (!game->nitros[i].active) {
                if (GetRandomValue(0, 800) < 1) {
                    int lane = GetRandomValue(0, 2);
                    game->nitros[i].active = true;
                    game->nitros[i].lane = lane;
                    game->nitros[i].rect = (Rectangle){ LANE_CENTERS[lane] - 12.0f, -50.0f, 24.0f, 30.0f };
                    game->nitros[i].speed = (float)GetRandomValue(80, 140);
                    game->nitros[i].color = (Color){ 0, 160, 255, 255 };
                }
            } else {
                game->nitros[i].rect.y += (game->gameSpeed - game->nitros[i].speed) * dt;
                if (game->nitros[i].rect.y > SCREEN_HEIGHT) game->nitros[i].active = false;

                if (CheckCollisionRecs(game->player.rect, game->nitros[i].rect)) {
                    game->nitros[i].active = false;
                    PlayNitroSound();
                    game->nitroTimer = 6.0f;
                }
            }
        }

        // --- INVISIBLE ITEMS (GREEN) ---
        for (int i = 0; i < MAX_INVISIBLES; i++) {
            if (!game->invisibles[i].active) {
                if (GetRandomValue(0, 900) < 1) {
                    int lane = GetRandomValue(0, 2);
                    game->invisibles[i].active = true;
                    game->invisibles[i].lane = lane;
                    game->invisibles[i].rect = (Rectangle){ LANE_CENTERS[lane] - 12.0f, -50.0f, 24.0f, 30.0f };
                    game->invisibles[i].speed = (float)GetRandomValue(80, 140);
                    game->invisibles[i].color = (Color){ 46, 204, 113, 255 };
                }
            } else {
                game->invisibles[i].rect.y += (game->gameSpeed - game->invisibles[i].speed) * dt;
                if (game->invisibles[i].rect.y > SCREEN_HEIGHT) game->invisibles[i].active = false;

                if (CheckCollisionRecs(game->player.rect, game->invisibles[i].rect)) {
                    game->invisibles[i].active = false;
                    PlayInvisibleSound();
                    game->invisibleTimer = 7.0f;
                }
            }
        }

        // --- POLICE & AMBULANCE ---
        if (!game->police.active && !game->ambulance.active && (IsKeyPressed(KEY_P) || game->floatScore >= game->police.targetScoreThreshold)) {
            game->police.active = true;
            PlayPoliceSirenSound();
            game->police.hitCount = 0;
            game->police.startScore = game->floatScore;
            game->police.speed = game->gameSpeed - 100.0f;
            game->police.rect.x = game->player.rect.x;
            game->police.rect.y = SCREEN_HEIGHT + 60.0f;

            if (game->floatScore >= game->police.targetScoreThreshold) {
                game->police.targetScoreThreshold += 10000.0f;
            }
        }

        if (game->police.active) {
            bool isRetreating = (game->floatScore - game->police.startScore >= 4000.0f);

            if (isRetreating) {
                float desiredPoliceSpeed = game->gameSpeed - 200.0f;
                game->police.speed = SmoothLerp(game->police.speed, desiredPoliceSpeed, 3.5f * dt);
                game->police.rect.y += (game->gameSpeed - game->police.speed) * dt;

                if (game->police.rect.y > SCREEN_HEIGHT + 80.0f) {
                    game->police.active = false;
                    FadePoliceSirenSound();
                }
            } else {
                float targetY = game->player.rect.y + 65.0f;
                float desiredPoliceSpeed = game->gameSpeed;
                if (game->police.rect.y > targetY + 10.0f) {
                    desiredPoliceSpeed = game->gameSpeed + 120.0f;
                } else if (game->police.rect.y < targetY - 10.0f) {
                    desiredPoliceSpeed = game->gameSpeed - 60.0f;
                }

                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (game->enemies[i].active) {
                        float distX = fabsf(game->police.rect.x - game->enemies[i].rect.x);
                        float distY = game->police.rect.y - game->enemies[i].rect.y;

                        if (distX < 25.0f && distY > 0.0f && distY < 75.0f) {
                            if (desiredPoliceSpeed > game->enemies[i].speed) {
                                desiredPoliceSpeed = game->enemies[i].speed - 10.0f;
                            }
                        }
                    }
                }

                if (desiredPoliceSpeed > 750.0f) desiredPoliceSpeed = 750.0f;

                game->police.speed = SmoothLerp(game->police.speed, desiredPoliceSpeed, 3.5f * dt);
                game->police.rect.y += (game->gameSpeed - game->police.speed) * dt;
                game->police.rect.x = SmoothLerp(game->police.rect.x, game->player.rect.x, 5.0f * dt);

                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (game->enemies[i].active) {
                        if (CheckCollisionRecs(game->police.rect, game->enemies[i].rect)) {
                            if (game->police.rect.y > game->enemies[i].rect.y) {
                                game->police.rect.y = game->enemies[i].rect.y + game->enemies[i].rect.height + 1.0f;
                                game->police.speed = game->enemies[i].speed;
                            }
                        }
                    }
                }

                if (game->currentState == PLAYING && CheckCollisionRecs(game->player.rect, game->police.rect)) {
                    game->police.hitCount++;

                    if (game->police.hitCount >= 2) {
                        PlayCrashSound();
                        if ((int)game->floatScore > game->highScore) {
                            game->highScore = (int)game->floatScore;
                        }
                        game->currentState = GAME_OVER;
                        SaveUserDataToCSV(game);
                        game->police.active = false;
                        FadePoliceSirenSound();
                    } else {
                        PlayBreakSound();
                        game->currentState = SKIDDING;
                        game->player.skidTimer = 0.9f;
                        game->player.skidDirection = (game->player.rect.x < game->police.rect.x) ? -1 : 1;
                        game->police.speed = game->gameSpeed - 150.0f;
                    }
                }
            }
        }

        if (!game->ambulance.active && game->floatScore >= 5000.0f && (IsKeyPressed(KEY_A) || GetRandomValue(0, 2500) < 2)) {
            int playerLane = 1;
            float minDiff = 999.0f;
            for (int l = 0; l < 3; l++) {
                float diff = fabsf((game->player.rect.x + 15.0f) - LANE_CENTERS[l]);
                if (diff < minDiff) {
                    minDiff = diff;
                    playerLane = l;
                }
            }

            game->ambulance.active = true;
            game->ambulance.currentLane = playerLane;
            game->ambulance.targetLane = playerLane;
            game->ambulance.isShifting = false;
            game->ambulance.shiftCooldown = 0.0f;
            
            game->ambulance.rect = (Rectangle){ LANE_CENTERS[playerLane] - 16.0f, SCREEN_HEIGHT + 220.0f, 32.0f, 58.0f };
            game->ambulance.speed = game->gameSpeed + 120.0f;
            
            SetAmbulanceSirenVolume(0.0f);
            PlayAmbulanceSirenSound();
        }

        if (game->ambulance.active) {
            game->ambulance.shiftCooldown -= dt;

            float ambY = game->ambulance.rect.y;
            float sirenVolume = 1.0f;

            if (ambY > SCREEN_HEIGHT) {
                float approachProgress = (SCREEN_HEIGHT + 220.0f - ambY) / 220.0f;
                sirenVolume = ClampFloat(approachProgress, 0.0f, 1.0f);
            } else if (ambY < 0.0f) {
                float departureProgress = (ambY - (-100.0f)) / 100.0f;
                sirenVolume = ClampFloat(departureProgress, 0.0f, 1.0f);
            }

            SetAmbulanceSirenVolume(sirenVolume);

            float currentClearance = GetLaneClearanceAhead(game, game->ambulance.targetLane, ambY);
            float targetSpeed = game->gameSpeed + 120.0f;

            if (currentClearance < 220.0f && game->ambulance.shiftCooldown <= 0.0f) {
                int leftLane = game->ambulance.targetLane - 1;
                int rightLane = game->ambulance.targetLane + 1;

                float leftClearance = (leftLane >= 0) ? GetLaneClearanceAhead(game, leftLane, ambY) : -1.0f;
                float rightClearance = (rightLane <= 2) ? GetLaneClearanceAhead(game, rightLane, ambY) : -1.0f;

                int bestLane = game->ambulance.targetLane;
                float bestClearance = currentClearance;

                if (leftClearance > bestClearance + 60.0f) {
                    bestClearance = leftClearance;
                    bestLane = leftLane;
                }
                if (rightClearance > bestClearance + 60.0f) {
                    bestClearance = rightClearance;
                    bestLane = rightLane;
                }

                if (bestLane != game->ambulance.targetLane) {
                    game->ambulance.targetLane = bestLane;
                    game->ambulance.isShifting = true;
                    game->ambulance.shiftCooldown = 0.35f;
                } else if (currentClearance < 110.0f) {
                    targetSpeed = game->gameSpeed + 15.0f;
                }
            }

            float speedLerpRate = (targetSpeed < game->ambulance.speed) ? 1.5f : 1.0f;
            game->ambulance.speed = SmoothLerp(game->ambulance.speed, targetSpeed, speedLerpRate * dt);
            game->ambulance.rect.y -= (game->ambulance.speed - game->gameSpeed) * dt;

            float targetX = LANE_CENTERS[game->ambulance.targetLane] - (game->ambulance.rect.width / 2.0f);
            game->ambulance.rect.x = SmoothLerp(game->ambulance.rect.x, targetX, 2.5f * dt);

            if (fabsf(game->ambulance.rect.x - targetX) < 1.5f) {
                game->ambulance.currentLane = game->ambulance.targetLane;
                game->ambulance.isShifting = false;
            }

            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (game->enemies[i].active) {
                    if (CheckCollisionRecs(game->ambulance.rect, game->enemies[i].rect)) {
                        if (game->ambulance.rect.y > game->enemies[i].rect.y) {
                            game->ambulance.rect.y = game->enemies[i].rect.y + game->enemies[i].rect.height + 2.0f;
                            game->ambulance.speed = game->enemies[i].speed;
                        }
                    }
                }
            }

            if (ambY < -100.0f) {
                game->ambulance.active = false;
                StopAmbulanceSirenSound();
            }

            if (game->currentState == PLAYING && CheckCollisionRecs(game->player.rect, game->ambulance.rect)) {
                PlayBreakSound();
                game->currentState = SKIDDING;
                game->player.skidTimer = 0.9f;
                game->player.skidDirection = (game->player.rect.x < game->ambulance.rect.x) ? -1 : 1;
            }
        }

        // --- ENEMIES ---
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (!game->enemies[i].active) {
                if (GetRandomValue(0, 100) < 2) {
                    int spawnLane = GetRandomValue(0, 2);

                    bool laneBlocked = false;
                    for (int j = 0; j < MAX_ENEMIES; j++) {
                        if (game->enemies[j].active && game->enemies[j].targetLane == spawnLane && game->enemies[j].rect.y < 120.0f) {
                            laneBlocked = true;
                            break;
                        }
                    }

                    if (!laneBlocked) {
                        game->enemies[i].active = true;
                        game->enemies[i].currentLane = spawnLane;
                        game->enemies[i].targetLane = spawnLane;
                        game->enemies[i].isShifting = false;
                        game->enemies[i].laneShiftTimer = (float)GetRandomValue(3, 7);
                        
                        int typeRandRoll = GetRandomValue(0, 9);
                        int typeRand;
                        if (typeRandRoll < 3) typeRand = ENEMY_CAR_BLUE;
                        else if (typeRandRoll < 6) typeRand = ENEMY_CAR_BLACK;
                        else if (typeRandRoll < 9) typeRand = ENEMY_CAR_YELLOW;
                        else typeRand = ENEMY_LORRY;
                        
                        game->enemies[i].type = typeRand;

                        if (typeRand == ENEMY_CAR_BLUE) {
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 15.0f, -60.0f, 30.0f, 50.0f };
                            game->enemies[i].speed = (float)GetRandomValue(100, 180);
                            game->enemies[i].color = (Color){ 50, 130, 220, 255 };
                        } else if (typeRand == ENEMY_CAR_BLACK) {
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 15.0f, -60.0f, 30.0f, 50.0f };
                            game->enemies[i].speed = (float)GetRandomValue(100, 170);
                            game->enemies[i].color = (Color){ 30, 30, 35, 255 };
                        } else if (typeRand == ENEMY_CAR_YELLOW) {
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 15.0f, -60.0f, 30.0f, 50.0f };
                            game->enemies[i].speed = (float)GetRandomValue(100, 175);
                            game->enemies[i].color = (Color){ 240, 200, 50, 255 };
                        } else if (typeRand == ENEMY_LORRY) {
                            game->enemies[i].rect = (Rectangle){ LANE_CENTERS[spawnLane] - 18.0f, -90.0f, 36.0f, 85.0f };
                            game->enemies[i].speed = (float)GetRandomValue(80, 120);
                            game->enemies[i].color = (Color){ 200, 70, 70, 255 };
                        }
                    }
                }
            } else {
                game->enemies[i].rect.y += (game->gameSpeed - game->enemies[i].speed) * dt;
                if (game->enemies[i].rect.y > SCREEN_HEIGHT) game->enemies[i].active = false;

                if (game->ambulance.active && !game->enemies[i].isShifting) {
                    if (game->ambulance.targetLane == game->enemies[i].currentLane &&
                        game->ambulance.rect.y > game->enemies[i].rect.y &&
                        (game->ambulance.rect.y - game->enemies[i].rect.y) < 250.0f) {
                        
                        int currentLane = game->enemies[i].currentLane;
                        int candidateLanes[2];
                        int candidateCount = 0;
                        if (currentLane - 1 >= 0) candidateLanes[candidateCount++] = currentLane - 1;
                        if (currentLane + 1 <= 2) candidateLanes[candidateCount++] = currentLane + 1;

                        for (int c = 0; c < candidateCount; c++) {
                            int targetL = candidateLanes[c];
                            bool laneClear = true;

                            for (int j = 0; j < MAX_ENEMIES; j++) {
                                if (i != j && game->enemies[j].active) {
                                    if (game->enemies[j].targetLane == targetL &&
                                        fabsf(game->enemies[i].rect.y - game->enemies[j].rect.y) < 90.0f) {
                                        laneClear = false;
                                    }
                                }
                            }

                            if (laneClear) {
                                game->enemies[i].targetLane = targetL;
                                game->enemies[i].isShifting = true;
                                break;
                            }
                        }
                    }
                }

                if (game->enemies[i].isShifting) {
                    float targetX = LANE_CENTERS[game->enemies[i].targetLane] - (game->enemies[i].rect.width / 2.0f);
                    game->enemies[i].rect.x = SmoothLerp(game->enemies[i].rect.x, targetX, 4.0f * dt);
                    if (fabsf(game->enemies[i].rect.x - targetX) < 1.0f) {
                        game->enemies[i].currentLane = game->enemies[i].targetLane;
                        game->enemies[i].isShifting = false;
                    }
                }

                if (game->currentState == PLAYING && CheckCollisionRecs(game->player.rect, game->enemies[i].rect)) {
                    if (game->invisibleTimer <= 0.0f) {
                        PlayBreakSound();
                        game->currentState = SKIDDING;
                        game->player.skidTimer = 0.9f;
                        game->player.skidDirection = (game->player.rect.x < game->enemies[i].rect.x) ? -1 : 1;
                    }
                }
            }
        }
    }
}