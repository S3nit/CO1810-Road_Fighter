#ifndef GAME_H
#define GAME_H

#include "raylib.h"
#include <stdbool.h>
#include <string.h>

#define MAX_ENEMIES 5
#define MAX_FUELS 3

// --- ENUMS ---
typedef enum { MENU, PLAYING, SKIDDING, GAME_OVER } GameState;

typedef enum {
    ENEMY_CAR_BLUE,
    ENEMY_CAR_BLACK,
    ENEMY_CAR_YELLOW,
    ENEMY_LORRY
} EnemyType;

// --- CONSTANTS ---
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern const float LANE_CENTERS[3];

// --- DATA STRUCTURES ---
typedef struct {
    Rectangle rect;
    float speed;
    Color color;
    bool active;
    int currentLane;
    int targetLane;
    float laneShiftTimer;
    bool isShifting;
    EnemyType type;
} Enemy;

typedef struct {
    Rectangle rect;
    float speed;
    Color color;
    bool active;
    int lane;
} FuelItem;

typedef struct {
    Rectangle rect;
    bool active;
    int hitCount;
    float targetScoreThreshold;
    float speed;
    float startScore;
} PoliceCar;

typedef struct {
    Rectangle rect;
    bool active;
    float speed;
    int currentLane;
    int targetLane;
    bool isShifting;
    float shiftCooldown;
} Ambulance;

typedef struct {
    Rectangle rect;
    float velocityX;
    Color color;
    float skidTimer;
    int skidDirection;
} Player;

typedef struct {
    GameState currentState;
    bool exitRequested;
    Player player;
    Enemy enemies[MAX_ENEMIES];
    FuelItem fuels[MAX_FUELS];
    PoliceCar police;
    Ambulance ambulance;
    float roadOffset;
    float gameSpeed;
    float floatScore;
    float fuel;
    char playerName[32];
    int highScore;
    int attempts;
    bool editingName;
} GameContext;

// --- LOGIC FUNCTIONS ---
void InitGame(GameContext *game);
void UpdateGame(GameContext *game, float dt);
void LoadUserDataFromCSV(GameContext *game);
void SaveUserDataToCSV(const GameContext *game);

#endif // GAME_H