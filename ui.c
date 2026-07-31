#include "ui.h"
#include "audiomanager.h"
#include "raylib.h"
#include <math.h>
#include <string.h>

static const Color COLOR_ASPHALT      = (Color){ 28, 31, 38, 255 };
static const Color COLOR_GRASS        = (Color){ 20, 64, 35, 255 };
static const Color COLOR_CURB         = (Color){ 55, 63, 75, 255 };
static const Color COLOR_UI_PANEL     = (Color){ 14, 16, 22, 242 };
static const Color COLOR_UI_BORDER    = (Color){ 45, 60, 85, 255 };
static const Color COLOR_GLASS        = (Color){ 90, 150, 210, 220 };
static const Color COLOR_WHEEL        = (Color){ 15, 17, 22, 255 };

// --- CUSTOM FONT HELPER (PC System Font) ---
static Font gameFont;
static bool fontInitialized = false;

static void InitGameFont(void) {
    if (!fontInitialized) {
        gameFont = LoadFontEx("C:/Windows/Fonts/arial.ttf", 40, 0, 0);
        SetTextureFilter(gameFont.texture, TEXTURE_FILTER_BILINEAR);
        fontInitialized = true;
    }
}

static void DrawCustomText(const char *text, Vector2 position, float fontSize, float spacing, Color tint) {
    InitGameFont();
    DrawTextEx(gameFont, text, position, fontSize, spacing, tint);
}

static Vector2 MeasureCustomText(const char *text, float fontSize, float spacing) {
    InitGameFont();
    return MeasureTextEx(gameFont, text, fontSize, spacing);
}

// Menu Navigation States & Selections
typedef enum {
    MENU_STATE_MAIN = 0,
    MENU_STATE_INSTRUCTIONS,
    MENU_STATE_SETTINGS,
    MENU_STATE_PROFILE,
    MENU_STATE_AUDIO,
    MENU_STATE_NAME_INPUT,
    MENU_STATE_EXIT_CONFIRM
} MenuState;

static MenuState currentMenuState = MENU_STATE_MAIN;
static int mainSelectedOption = 0;       
static int settingsSelectedOption = 0;   
static int profileSelectedOption = 0;    
static int audioSelectedOption = 0;      
static int exitConfirmOption = 1;        // 0: Yes, 1: No
static int masterVolumePercent = 80;     
static int pauseSelectedOption = 0;      // 0: Resume, 1: Exit to Menu

// Track previous options for hover sound triggers
static int prevMainSelected = 0;
static int prevSettingsSelected = 0;
static int prevAudioSelected = 0;
static int prevExitConfirmSelected = 1;

void UpdateMenuLogic(GameContext *game) {
    Vector2 mousePos = GetMousePosition();

    if (currentMenuState == MENU_STATE_MAIN) {
        prevMainSelected = mainSelectedOption;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            mainSelectedOption = (mainSelectedOption + 1) % 4;
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            mainSelectedOption = (mainSelectedOption - 1 + 4) % 4;
        }

        Rectangle btnStart = { (SCREEN_WIDTH - 260.0f) / 2.0f, 220.0f, 260.0f, 48.0f };
        Rectangle btnInstr = { (SCREEN_WIDTH - 260.0f) / 2.0f, 282.0f, 260.0f, 48.0f };
        Rectangle btnSett  = { (SCREEN_WIDTH - 260.0f) / 2.0f, 344.0f, 260.0f, 48.0f };
        Rectangle btnExit  = { (SCREEN_WIDTH - 260.0f) / 2.0f, 406.0f, 260.0f, 48.0f };

        if (CheckCollisionPointRec(mousePos, btnStart)) {
            mainSelectedOption = 0;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_NAME_INPUT;
                game->editingName = true;
                if (game->playerName[0] == '\0') {
                    strcpy(game->playerName, "Player");
                }
            }
        } else if (CheckCollisionPointRec(mousePos, btnInstr)) {
            mainSelectedOption = 1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_INSTRUCTIONS;
            }
        } else if (CheckCollisionPointRec(mousePos, btnSett)) {
            mainSelectedOption = 2;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_SETTINGS;
                settingsSelectedOption = 0;
            }
        } else if (CheckCollisionPointRec(mousePos, btnExit)) {
            mainSelectedOption = 3;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_EXIT_CONFIRM;
                exitConfirmOption = 1; // Default to 'No'
            }
        }

        if (mainSelectedOption != prevMainSelected) {
            PlayButtonHoverSound();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            PlayMenuSelectSound();
            if (mainSelectedOption == 0) {
                currentMenuState = MENU_STATE_NAME_INPUT;
                game->editingName = true;
                if (game->playerName[0] == '\0') {
                    strcpy(game->playerName, "Player");
                }
            } else if (mainSelectedOption == 1) {
                currentMenuState = MENU_STATE_INSTRUCTIONS;
            } else if (mainSelectedOption == 2) {
                currentMenuState = MENU_STATE_SETTINGS;
                settingsSelectedOption = 0;
            } else if (mainSelectedOption == 3) {
                currentMenuState = MENU_STATE_EXIT_CONFIRM;
                exitConfirmOption = 1;
            }
        }
    }
    else if (currentMenuState == MENU_STATE_EXIT_CONFIRM) {
        prevExitConfirmSelected = exitConfirmOption;
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_A) || IsKeyPressed(KEY_D)) {
            exitConfirmOption = 1 - exitConfirmOption;
        }

        Rectangle btnYes = { SCREEN_WIDTH / 2.0f - 110.0f, 305.0f, 90.0f, 36.0f };
        Rectangle btnNo  = { SCREEN_WIDTH / 2.0f + 20.0f,  305.0f, 90.0f, 36.0f };

        if (CheckCollisionPointRec(mousePos, btnYes)) {
            exitConfirmOption = 0;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                game->exitRequested = true;
            }
        } else if (CheckCollisionPointRec(mousePos, btnNo)) {
            exitConfirmOption = 1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_MAIN;
            }
        }

        if (exitConfirmOption != prevExitConfirmSelected) {
            PlayButtonHoverSound();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            PlayMenuSelectSound();
            if (exitConfirmOption == 0) {
                game->exitRequested = true;
            } else {
                currentMenuState = MENU_STATE_MAIN;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_MAIN;
        }
    }
    else if (currentMenuState == MENU_STATE_NAME_INPUT) {
        int key = GetCharPressed();
        while (key > 0) {
            if ((key >= 32) && (key <= 125) && (strlen(game->playerName) < 31)) {
                int len = strlen(game->playerName);
                game->playerName[len] = (char)key;
                game->playerName[len + 1] = '\0';
            }
            key = GetCharPressed();
        }
        if (IsKeyPressed(KEY_BACKSPACE)) {
            int len = strlen(game->playerName);
            if (len > 0) {
                game->playerName[len - 1] = '\0';
            }
        }
        if (IsKeyPressed(KEY_ENTER)) {
            PlayMenuSelectSound();
            game->editingName = false;
            LoadUserDataFromCSV(game);
            game->attempts++;
            SaveUserDataToCSV(game);

            game->currentState = PLAYING;
            currentMenuState = MENU_STATE_MAIN;
            StopMenuBGM();
            PlayBGM();
        }

        Rectangle startBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 355.0f, 200.0f, 40.0f };
        if (CheckCollisionPointRec(mousePos, startBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlayMenuSelectSound();
            game->editingName = false;
            LoadUserDataFromCSV(game);
            game->attempts++;
            SaveUserDataToCSV(game);

            game->currentState = PLAYING;
            currentMenuState = MENU_STATE_MAIN;
            StopMenuBGM();
            PlayBGM();
        }
    }
    else if (currentMenuState == MENU_STATE_INSTRUCTIONS) {
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_MAIN;
        }
        Rectangle backBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 495.0f, 200.0f, 42.0f };
        if (CheckCollisionPointRec(mousePos, backBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_MAIN;
        }
    }
    else if (currentMenuState == MENU_STATE_SETTINGS) {
        prevSettingsSelected = settingsSelectedOption;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
            settingsSelectedOption = (settingsSelectedOption + 1) % 3;
        }
        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            settingsSelectedOption = (settingsSelectedOption - 1 + 3) % 3;
        }

        Rectangle btnProfile = { (SCREEN_WIDTH - 260.0f) / 2.0f, 350.0f, 260.0f, 46.0f };
        Rectangle btnAudio   = { (SCREEN_WIDTH - 260.0f) / 2.0f, 410.0f, 260.0f, 46.0f };
        Rectangle btnBack    = { (SCREEN_WIDTH - 260.0f) / 2.0f, 470.0f, 260.0f, 46.0f };

        if (CheckCollisionPointRec(mousePos, btnProfile)) {
            settingsSelectedOption = 0;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_PROFILE;
                profileSelectedOption = 0;
            }
        } else if (CheckCollisionPointRec(mousePos, btnAudio)) {
            settingsSelectedOption = 1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_AUDIO;
                audioSelectedOption = 0;
            }
        } else if (CheckCollisionPointRec(mousePos, btnBack)) {
            settingsSelectedOption = 2;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_MAIN;
            }
        }

        if (settingsSelectedOption != prevSettingsSelected) {
            PlayButtonHoverSound();
        }

        if (IsKeyPressed(KEY_ENTER)) {
            PlayMenuSelectSound();
            if (settingsSelectedOption == 0) {
                currentMenuState = MENU_STATE_PROFILE;
                profileSelectedOption = 0;
            } else if (settingsSelectedOption == 1) {
                currentMenuState = MENU_STATE_AUDIO;
                audioSelectedOption = 0;
            } else if (settingsSelectedOption == 2) {
                currentMenuState = MENU_STATE_MAIN;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_MAIN;
        }
    }
    else if (currentMenuState == MENU_STATE_PROFILE) {
        if (game->editingName) {
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= 32) && (key <= 125) && (strlen(game->playerName) < 31)) {
                    int len = strlen(game->playerName);
                    game->playerName[len] = (char)key;
                    game->playerName[len + 1] = '\0';
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE)) {
                int len = strlen(game->playerName);
                if (len > 0) {
                    game->playerName[len - 1] = '\0';
                }
            }
            if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_ESCAPE)) {
                PlayMenuSelectSound();
                game->editingName = false;
                SaveUserDataToCSV(game);
            }
        } else {
            if (IsKeyPressed(KEY_E) || IsKeyPressed(KEY_ENTER)) {
                PlayMenuSelectSound();
                game->editingName = true;
            }
            Rectangle nameRect = { 48.0f, 175.0f, 250.0f, 25.0f };
            if (CheckCollisionPointRec(mousePos, nameRect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                game->editingName = true;
            }
        }

        Rectangle backBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 480.0f, 200.0f, 40.0f };
        if (CheckCollisionPointRec(mousePos, backBtn) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            PlayMenuSelectSound();
            game->editingName = false;
            currentMenuState = MENU_STATE_SETTINGS;
        }
        if (IsKeyPressed(KEY_ESCAPE) && !game->editingName) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_SETTINGS;
        }
    }
    else if (currentMenuState == MENU_STATE_AUDIO) {
        prevAudioSelected = audioSelectedOption;
        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            audioSelectedOption = (audioSelectedOption + 1) % 2;
        }
        if (audioSelectedOption == 0) {
            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
                if (masterVolumePercent < 100) masterVolumePercent += 10;
            }
            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
                if (masterVolumePercent > 0) masterVolumePercent -= 10;
            }
        }

        if (audioSelectedOption != prevAudioSelected) {
            PlayButtonHoverSound();
        }

        Rectangle btnBack = { (SCREEN_WIDTH - 260.0f) / 2.0f, 450.0f, 260.0f, 44.0f };
        if (CheckCollisionPointRec(mousePos, btnBack)) {
            audioSelectedOption = 1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                currentMenuState = MENU_STATE_SETTINGS;
            }
        }

        if (IsKeyPressed(KEY_ENTER) && audioSelectedOption == 1) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_SETTINGS;
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            currentMenuState = MENU_STATE_SETTINGS;
        }
    }
}

static void DrawCapsuleButton(Rectangle rect, const char *text, bool isSelected) {
    float pulse = isSelected ? (sinf((float)GetTime() * 8.0f) * 2.0f + 2.0f) : 0.0f;
    Rectangle animRect = {
        rect.x - pulse,
        rect.y - pulse * 0.5f,
        rect.width + (pulse * 2.0f),
        rect.height + pulse
    };

    Color fillColor = isSelected ? (Color){ 25, 45, 80, 255 } : (Color){ 20, 23, 30, 255 };
    Color borderColor = isSelected ? (Color){ 0, 200, 255, 255 } : (Color){ 45, 55, 75, 255 };

    if (isSelected) {
        DrawRectangleRounded(animRect, 1.0f, 8, Fade((Color){ 0, 180, 255, 255 }, 0.25f));
    }

    DrawRectangleRounded(animRect, 1.0f, 8, fillColor);
    DrawRectangleRoundedLines(animRect, 1.0f, 8, borderColor);

    float fontSize = 14.0f;
    Vector2 textSize = MeasureCustomText(text, fontSize, 1.0f);
    float textX = animRect.x + (animRect.width - textSize.x) / 2.0f;
    float textY = animRect.y + (animRect.height - textSize.y) / 2.0f;

    if (isSelected) {
        DrawCustomText(">", (Vector2){ textX - 18.0f, textY }, fontSize, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawCustomText("<", (Vector2){ textX + textSize.x + 8.0f, textY }, fontSize, 1.0f, (Color){ 0, 200, 255, 255 });
    }

    DrawCustomText(text, (Vector2){ textX, textY }, fontSize, 1.0f, isSelected ? WHITE : (Color){ 150, 165, 185, 255 });
}

void DrawMenuScreen(const GameContext *game) {
    BeginDrawing();
    ClearBackground((Color){ 10, 12, 17, 255 });

    for (int i = 0; i < SCREEN_HEIGHT; i += 30) {
        DrawLine(0, i, SCREEN_WIDTH, i, Fade(WHITE, 0.015f));
    }
    for (int i = 0; i < SCREEN_WIDTH; i += 40) {
        DrawLine(i, 0, i, SCREEN_HEIGHT, Fade(WHITE, 0.015f));
    }

    const char *titleText = "ROAD FIGHTER";
    float titleFontSize = 32.0f;
    Vector2 titleSize = MeasureCustomText(titleText, titleFontSize, 1.5f);
    DrawCustomText(titleText, (Vector2){ SCREEN_WIDTH / 2.0f - titleSize.x / 2.0f + 2.0f, 53.0f }, titleFontSize, 1.5f, Fade((Color){ 255, 45, 85, 255 }, 0.6f));
    DrawCustomText(titleText, (Vector2){ SCREEN_WIDTH / 2.0f - titleSize.x / 2.0f, 50.0f }, titleFontSize, 1.5f, (Color){ 245, 50, 75, 255 });

    const char *subtitleText = "TURBO ARCADE EDITION";
    float subFontSize = 11.0f;
    Vector2 subSize = MeasureCustomText(subtitleText, subFontSize, 1.0f);
    DrawCustomText(subtitleText, (Vector2){ SCREEN_WIDTH / 2.0f - subSize.x / 2.0f, 90.0f }, subFontSize, 1.0f, (Color){ 0, 200, 255, 255 });

    if (currentMenuState == MENU_STATE_MAIN) {
        Rectangle btnStart = { (SCREEN_WIDTH - 260.0f) / 2.0f, 220.0f, 260.0f, 48.0f };
        Rectangle btnInstr = { (SCREEN_WIDTH - 260.0f) / 2.0f, 282.0f, 260.0f, 48.0f };
        Rectangle btnSett  = { (SCREEN_WIDTH - 260.0f) / 2.0f, 344.0f, 260.0f, 48.0f };
        Rectangle btnExit  = { (SCREEN_WIDTH - 260.0f) / 2.0f, 406.0f, 260.0f, 48.0f };

        DrawCapsuleButton(btnStart, "START GAME", mainSelectedOption == 0);
        DrawCapsuleButton(btnInstr, "INSTRUCTIONS", mainSelectedOption == 1);
        DrawCapsuleButton(btnSett,  "SETTINGS",     mainSelectedOption == 2);
        DrawCapsuleButton(btnExit,  "EXIT GAME",    mainSelectedOption == 3);
    }
    else if (currentMenuState == MENU_STATE_EXIT_CONFIRM) {
        Rectangle exitBox = { 30, 180, SCREEN_WIDTH - 60, 200 };
        DrawRectangleRounded(exitBox, 0.12f, 6, (Color){ 18, 22, 30, 240 });
        DrawRectangleRoundedLines(exitBox, 0.12f, 6, (Color){ 231, 76, 60, 255 });

        DrawCustomText("EXIT CONFIRMATION", (Vector2){ 48.0f, 200.0f }, 12.0f, 1.0f, (Color){ 231, 76, 60, 255 });
        DrawLine(48, 218, SCREEN_WIDTH - 48, 218, Fade(WHITE, 0.1f));

        DrawCustomText("Do you want to leave?", (Vector2){ SCREEN_WIDTH / 2.0f - MeasureCustomText("Do you want to leave?", 12.0f, 1.0f).x / 2.0f, 250.0f }, 12.0f, 1.0f, WHITE);

        Rectangle btnYes = { SCREEN_WIDTH / 2.0f - 110.0f, 305.0f, 90.0f, 36.0f };
        Rectangle btnNo  = { SCREEN_WIDTH / 2.0f + 20.0f,  305.0f, 90.0f, 36.0f };

        DrawCapsuleButton(btnYes, "YES", exitConfirmOption == 0);
        DrawCapsuleButton(btnNo,  "NO",  exitConfirmOption == 1);
    }
    else if (currentMenuState == MENU_STATE_NAME_INPUT) {
        Rectangle nameBox = { 30, 140, SCREEN_WIDTH - 60, 280 };
        DrawRectangleRounded(nameBox, 0.08f, 6, (Color){ 18, 22, 30, 240 });
        DrawRectangleRoundedLines(nameBox, 0.08f, 6, (Color){ 45, 65, 95, 255 });

        DrawCustomText("ENTER DRIVER NAME", (Vector2){ 48.0f, 160.0f }, 12.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine(48, 178, SCREEN_WIDTH - 48, 178, Fade(WHITE, 0.1f));

        DrawCustomText("Please enter your name:", (Vector2){ 48.0f, 210.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        
        Rectangle inputField = { 48.0f, 245.0f, SCREEN_WIDTH - 96.0f, 40.0f };
        DrawRectangleRounded(inputField, 0.3f, 4, (Color){ 25, 30, 40, 255 });
        DrawRectangleRoundedLines(inputField, 0.3f, 4, GOLD);

        DrawCustomText(game->playerName, (Vector2){ inputField.x + 15.0f, inputField.y + 12.0f }, 12.0f, 1.0f, WHITE);

        DrawCustomText("Type name & press [ENTER] to start", (Vector2){ 48.0f, 310.0f }, 10.0f, 1.0f, (Color){ 110, 130, 150, 255 });

        Rectangle startBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 355.0f, 200.0f, 40.0f };
        DrawCapsuleButton(startBtn, "START RACE", true);
    }
    else if (currentMenuState == MENU_STATE_INSTRUCTIONS) {
        Rectangle instrBox = { 30, 115, SCREEN_WIDTH - 60, 365 };
        DrawRectangleRounded(instrBox, 0.08f, 6, (Color){ 18, 22, 30, 240 });
        DrawRectangleRoundedLines(instrBox, 0.08f, 6, (Color){ 45, 65, 95, 255 });

        DrawCustomText("DRIVER MANUAL", (Vector2){ 48.0f, 132.0f }, 12.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine(48, 148, SCREEN_WIDTH - 48, 148, Fade(WHITE, 0.1f));

        DrawCustomText("A / D or Arrows", (Vector2){ 48.0f, 165.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Steer Vehicle", (Vector2){ 185.0f, 165.0f }, 11.0f, 1.0f, WHITE);

        DrawCustomText("UP / DOWN", (Vector2){ 48.0f, 192.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Speed Control", (Vector2){ 185.0f, 192.0f }, 11.0f, 1.0f, WHITE);

        DrawCustomText("P Key", (Vector2){ 48.0f, 219.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Request Police Escort", (Vector2){ 185.0f, 219.0f }, 11.0f, 1.0f, (Color){ 100, 180, 255, 255 });

        DrawCustomText("Pink Fuel [ F ]", (Vector2){ 48.0f, 246.0f }, 11.0f, 1.0f, (Color){ 240, 80, 160, 255 });
        DrawCustomText("Replenish Gas Supply", (Vector2){ 185.0f, 246.0f }, 11.0f, 1.0f, (Color){ 240, 80, 160, 255 });

        DrawCustomText("Blue Nitro [ N ]", (Vector2){ 48.0f, 273.0f }, 11.0f, 1.0f, (Color){ 0, 180, 255, 255 });
        DrawCustomText("Super Speed Boost", (Vector2){ 185.0f, 273.0f }, 11.0f, 1.0f, (Color){ 0, 180, 255, 255 });

        DrawCustomText("Green Stealth [ I ]", (Vector2){ 48.0f, 300.0f }, 11.0f, 1.0f, (Color){ 46, 204, 113, 255 });
        DrawCustomText("Ghost Invisibility", (Vector2){ 185.0f, 300.0f }, 11.0f, 1.0f, (Color){ 46, 204, 113, 255 });

        DrawCustomText("Avoid Traffic", (Vector2){ 48.0f, 327.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Prevent Collisions", (Vector2){ 185.0f, 327.0f }, 11.0f, 1.0f, (Color){ 231, 76, 60, 255 });

        Rectangle backBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 495.0f, 200.0f, 40.0f };
        DrawCapsuleButton(backBtn, "BACK TO MENU", true);
    }
    else if (currentMenuState == MENU_STATE_SETTINGS) {
        Rectangle btnProfile = { (SCREEN_WIDTH - 260.0f) / 2.0f, 350.0f, 260.0f, 46.0f };
        Rectangle btnAudio   = { (SCREEN_WIDTH - 260.0f) / 2.0f, 410.0f, 260.0f, 46.0f };
        Rectangle btnBack    = { (SCREEN_WIDTH - 260.0f) / 2.0f, 470.0f, 260.0f, 46.0f };

        DrawCapsuleButton(btnProfile, "PROFILE",       settingsSelectedOption == 0);
        DrawCapsuleButton(btnAudio,   "AUDIO SETTINGS", settingsSelectedOption == 1);
        DrawCapsuleButton(btnBack,    "BACK",           settingsSelectedOption == 2);

        Rectangle settBox = { 30, 130, SCREEN_WIDTH - 60, 190 };
        DrawRectangleRounded(settBox, 0.12f, 6, (Color){ 18, 22, 30, 230 });
        DrawRectangleRoundedLines(settBox, 0.12f, 6, (Color){ 45, 65, 95, 255 });

        DrawCustomText("SYSTEM CONFIGURATION", (Vector2){ 48.0f, 150.0f }, 12.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine(48, 166, SCREEN_WIDTH - 48, 166, Fade(WHITE, 0.1f));
        DrawCustomText("Customize your profile credentials", (Vector2){ 48.0f, 185.0f }, 11.0f, 1.0f, (Color){ 150, 165, 185, 255 });
        DrawCustomText("and adjust immersive sound parameters.", (Vector2){ 48.0f, 210.0f }, 11.0f, 1.0f, (Color){ 150, 165, 185, 255 });
    }
    else if (currentMenuState == MENU_STATE_PROFILE) {
        Rectangle profileBox = { 30, 120, SCREEN_WIDTH - 60, 360 };
        DrawRectangleRounded(profileBox, 0.08f, 6, (Color){ 18, 22, 30, 240 });
        DrawRectangleRoundedLines(profileBox, 0.08f, 6, (Color){ 45, 65, 95, 255 });

        DrawCustomText("DRIVER PROFILE", (Vector2){ 48.0f, 138.0f }, 12.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine(48, 154, SCREEN_WIDTH - 48, 154, Fade(WHITE, 0.1f));

        DrawCustomText("Name:", (Vector2){ 48.0f, 180.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText(game->playerName, (Vector2){ 170.0f, 180.0f }, 11.0f, 1.0f, game->editingName ? GOLD : WHITE);

        DrawCustomText("Highest Score:", (Vector2){ 48.0f, 215.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText(TextFormat("%06i", game->highScore), (Vector2){ 170.0f, 215.0f }, 11.0f, 1.0f, (Color){ 241, 196, 15, 255 });

        DrawCustomText("Attempts:", (Vector2){ 48.0f, 250.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText(TextFormat("%d", game->attempts), (Vector2){ 170.0f, 250.0f }, 11.0f, 1.0f, WHITE);

        DrawCustomText("Institution:", (Vector2){ 48.0f, 285.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Univ. of Peradeniya (UoP)", (Vector2){ 170.0f, 285.0f }, 11.0f, 1.0f, WHITE);

        DrawCustomText("Driver Status:", (Vector2){ 48.0f, 320.0f }, 11.0f, 1.0f, (Color){ 140, 155, 175, 255 });
        DrawCustomText("Elite S-Tier Racer", (Vector2){ 170.0f, 320.0f }, 11.0f, 1.0f, (Color){ 46, 204, 113, 255 });

        if (game->editingName) {
            DrawCustomText("Type name & press [ENTER]", (Vector2){ 48.0f, 355.0f }, 10.0f, 1.0f, GOLD);
        } else {
            DrawCustomText("Click Name or Press [E] to Edit Name", (Vector2){ 48.0f, 355.0f }, 10.0f, 1.0f, (Color){ 110, 130, 150, 255 });
        }

        Rectangle backBtn = { (SCREEN_WIDTH - 200.0f) / 2.0f, 480.0f, 200.0f, 40.0f };
        DrawCapsuleButton(backBtn, "BACK", true);
    }
    else if (currentMenuState == MENU_STATE_AUDIO) {
        Rectangle audioBox = { 30, 130, SCREEN_WIDTH - 60, 280 };
        DrawRectangleRounded(audioBox, 0.12f, 6, (Color){ 18, 22, 30, 230 });
        DrawRectangleRoundedLines(audioBox, 0.12f, 6, (Color){ 45, 65, 95, 255 });

        DrawCustomText("AUDIO SETTINGS", (Vector2){ 48.0f, 150.0f }, 12.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine(48, 166, SCREEN_WIDTH - 48, 166, Fade(WHITE, 0.1f));

        DrawCustomText("Master Volume:", (Vector2){ 48.0f, 195.0f }, 11.0f, 1.0f, (Color){ 150, 165, 185, 255 });
        DrawCustomText(TextFormat("<  %3d%%  >", masterVolumePercent), (Vector2){ 190.0f, 195.0f }, 11.0f, 1.0f, audioSelectedOption == 0 ? (Color){ 0, 200, 255, 255 } : WHITE);

        Rectangle barBg = { 48, 230, SCREEN_WIDTH - 96, 12 };
        DrawRectangleRounded(barBg, 0.5f, 4, (Color){ 25, 30, 40, 255 });
        Rectangle barFill = { 48, 230, (SCREEN_WIDTH - 96) * ((float)masterVolumePercent / 100.0f), 12 };
        DrawRectangleRounded(barFill, 0.5f, 4, (Color){ 0, 180, 255, 255 });

        DrawCustomText("Use LEFT / RIGHT arrows to adjust volume.", (Vector2){ 48.0f, 270.0f }, 10.0f, 1.0f, (Color){ 110, 130, 150, 255 });

        Rectangle btnBack = { (SCREEN_WIDTH - 260.0f) / 2.0f, 450.0f, 260.0f, 44.0f };
        DrawCapsuleButton(btnBack, "BACK", audioSelectedOption == 1);
    }

    EndDrawing();
}

static void DrawPlayerCar(const GameContext *game) {
    const Player *player = &game->player;
    Rectangle rect = player->rect;
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    // --- INVISIBILITY OPACITY & BLINK EFFECT ---
    float alphaMult = 1.0f;
    if (game->invisibleTimer > 0.0f) {
        bool blink = ((int)(GetTime() * 14.0f) % 2 == 0);
        alphaMult = blink ? 0.25f : 0.55f;
    }

    Color beamColor = Fade((Color){ 255, 245, 200, 255 }, 0.15f * alphaMult);
    Vector2 leftHeadlight = { x + 6.0f, y + 2.0f };
    Vector2 rightHeadlight = { x + w - 6.0f, y + 2.0f };

    DrawTriangle((Vector2){ x - 15.0f, y - 120.0f }, (Vector2){ x + 16.0f, y - 120.0f }, leftHeadlight, beamColor);
    DrawTriangle((Vector2){ x + w - 16.0f, y - 120.0f }, (Vector2){ x + w + 15.0f, y - 120.0f }, rightHeadlight, beamColor);

    bool isBraking = IsKeyDown(KEY_DOWN) && game->currentState == PLAYING;
    if (isBraking) {
        Color brakeBloom = Fade((Color){ 255, 30, 30, 255 }, 0.35f * alphaMult);
        DrawCircleGradient((Vector2){ x + 6.0f, y + h + 4.0f }, 16.0f, brakeBloom, BLANK);
        DrawCircleGradient((Vector2){ x + w - 6.0f, y + h + 4.0f }, 16.0f, brakeBloom, BLANK);
    }

    float steerOffset = (player->velocityX / 320.0f) * 3.0f;
    float wheelW = 5.0f;
    float wheelH = 11.0f;

    Color wheelColor = Fade(COLOR_WHEEL, alphaMult);
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f + steerOffset, y + 8.0f, wheelW, wheelH }, 0.4f, 2, wheelColor);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f + steerOffset, y + 8.0f, wheelW, wheelH }, 0.4f, 2, wheelColor);
    DrawRectangleRounded((Rectangle){ x - wheelW - 1.0f, y + h - 18.0f, wheelW + 1.0f, wheelH + 1.0f }, 0.4f, 2, wheelColor);
    DrawRectangleRounded((Rectangle){ x + w, y + h - 18.0f, wheelW + 1.0f, wheelH + 1.0f }, 0.4f, 2, wheelColor);

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 5.0f, w, h }, 0.3f, 4, Fade((Color){ 0, 0, 0, 100 }, alphaMult));

    Color bodyColor = player->color;
    if (game->currentState == SKIDDING && ((int)(GetTime() * 12) % 2 == 0)) {
        bodyColor = WHITE;
    }
    bodyColor = Fade(bodyColor, alphaMult);

    DrawRectangleRounded((Rectangle){ x - 2.0f, y + h * 0.50f, w + 4.0f, h * 0.42f }, 0.4f, 4, bodyColor);
    DrawRectangleRounded(rect, 0.35f, 6, bodyColor);
    DrawTriangle((Vector2){ x + 1.0f, y + 6.0f }, (Vector2){ x + w / 2.0f, y - 2.0f }, (Vector2){ x + w - 1.0f, y + 6.0f }, Fade((Color){ 18, 20, 25, 255 }, alphaMult));
    DrawTriangle((Vector2){ x + w / 2.0f - 5.0f, y + 16.0f }, (Vector2){ x + w / 2.0f + 5.0f, y + 16.0f }, (Vector2){ x + w / 2.0f, y + 24.0f }, Fade((Color){ 20, 22, 28, 255 }, alphaMult));

    Color glassColor = Fade(COLOR_GLASS, alphaMult);
    DrawTriangle((Vector2){ x + 5.0f, y + h * 0.36f }, (Vector2){ x + w / 2.0f, y + h * 0.22f }, (Vector2){ x + w - 5.0f, y + h * 0.36f }, glassColor);
    DrawRectangle((int)(x + 5.0f), (int)(y + h * 0.36f), (int)(w - 10.0f), (int)(h * 0.12f), glassColor);

    Rectangle carbonRoof = { x + 6.0f, y + h * 0.42f, w - 12.0f, h * 0.22f };
    DrawRectangleRounded(carbonRoof, 0.3f, 4, Fade((Color){ 22, 24, 30, 255 }, alphaMult));
    DrawRectangleRounded((Rectangle){ x + 7.0f, y + h * 0.62f, w - 14.0f, h * 0.10f }, 0.3f, 2, glassColor);

    DrawRectangleRounded((Rectangle){ x - 3.0f, y + h - 6.0f, w + 6.0f, 5.0f }, 0.5f, 2, Fade((Color){ 15, 17, 22, 255 }, alphaMult));
    DrawRectangle((int)(x - 4.0f), (int)(y + h - 8.0f), 3, 8, bodyColor);
    DrawRectangle((int)(x + w + 1.0f), (int)(y + h - 8.0f), 3, 8, bodyColor);

    DrawLineEx((Vector2){ x + 3.0f, y + 6.0f }, (Vector2){ x + 9.0f, y + 2.0f }, 2.0f, Fade(WHITE, alphaMult));
    DrawLineEx((Vector2){ x + w - 3.0f, y + 6.0f }, (Vector2){ x + w - 9.0f, y + 2.0f }, 2.0f, Fade(WHITE, alphaMult));

    Color tailColor = isBraking ? (Color){ 255, 40, 40, 255 } : (Color){ 200, 25, 25, 255 };
    tailColor = Fade(tailColor, alphaMult);
    DrawRectangleRounded((Rectangle){ x + 1.0f, y + h - 2.0f, w - 2.0f, 3.0f }, 0.5f, 2, tailColor);
}

static void DrawCarShape(Rectangle rect, Color bodyColor, bool isPolice, bool isIndicatorOn) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 4.0f, w, h }, 0.25f, 4, (Color){ 0, 0, 0, 80 });

    float wheelW = 4.0f;
    float wheelH = 10.0f;
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + 7.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + 7.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + h - 17.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + h - 17.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);

    if (isPolice) {
        DrawRectangleRounded(rect, 0.3f, 4, (Color){ 18, 20, 24, 255 });
        DrawRectangle((int)(x + 2.0f), (int)(y + h * 0.28f), (int)(w - 4.0f), (int)(h * 0.46f), WHITE);
    } else {
        DrawRectangleRounded(rect, 0.3f, 4, bodyColor);
    }

    DrawRectangle((int)(x + 5.0f), (int)(y + 1.0f), (int)(w - 10.0f), 3, (Color){ 25, 28, 35, 255 });
    DrawRectangle((int)(x - 2.0f), (int)(y + h * 0.28f), 2, 5, isPolice ? BLACK : bodyColor);
    DrawRectangle((int)(x + w), (int)(y + h * 0.28f), 2, 5, isPolice ? BLACK : bodyColor);

    DrawRectangleRounded((Rectangle){ x + 4.0f, y + h * 0.24f, w - 8.0f, h * 0.16f }, 0.3f, 2, COLOR_GLASS);
    
    Rectangle roof = { x + 4.0f, y + h * 0.42f, w - 8.0f, h * 0.24f };
    Color roofColor = isPolice ? WHITE : ColorAlpha(bodyColor, 0.85f);
    DrawRectangleRounded(roof, 0.15f, 2, roofColor);
    DrawRectangleRounded((Rectangle){ x + 5.0f, y + h * 0.68f, w - 10.0f, h * 0.12f }, 0.3f, 2, COLOR_GLASS);

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 2.0f, 6.0f, 3.0f }, 0.5f, 2, (Color){ 255, 245, 200, 255 });
    DrawRectangleRounded((Rectangle){ x + w - 8.0f, y + 2.0f, 6.0f, 3.0f }, 0.5f, 2, (Color){ 255, 245, 200, 255 });

    DrawRectangle((int)(x + 3.0f), (int)(y + h - 3.0f), 6, 2, (Color){ 220, 35, 35, 255 });
    DrawRectangle((int)(x + w - 9.0f), (int)(y + h - 3.0f), 6, 2, (Color){ 220, 35, 35, 255 });

    if (isIndicatorOn) {
        DrawCircleV((Vector2){ x + 1.0f, y + 3.0f }, 3.0f, GOLD);
        DrawCircleV((Vector2){ x + w - 1.0f, y + 3.0f }, 3.0f, GOLD);
    }

    if (isPolice) {
        bool toggle = ((int)(GetTime() * 12) % 2 == 0);
        Color leftLight = toggle ? (Color){ 240, 30, 30, 255 } : (Color){ 30, 30, 30, 255 };
        Color rightLight = !toggle ? (Color){ 30, 110, 255, 255 } : (Color){ 30, 30, 30, 255 };

        DrawRectangle((int)(x + w / 2.0f - 9.0f), (int)(y + h * 0.48f), 18, 5, DARKGRAY);
        DrawRectangle((int)(x + w / 2.0f - 8.0f), (int)(y + h * 0.48f + 1.0f), 7, 3, leftLight);
        DrawRectangle((int)(x + w / 2.0f + 1.0f), (int)(y + h * 0.48f + 1.0f), 7, 3, rightLight);
    }
}

static void DrawLorryShape(Rectangle rect, Color bodyColor, bool isIndicatorOn) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 4.0f, w, h }, 0.1f, 4, (Color){ 0, 0, 0, 80 });

    float wheelW = 5.0f;
    float wheelH = 14.0f;
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + 10.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + 10.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + h - 35.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + h - 35.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + h - 17.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + h - 17.0f, wheelW, wheelH }, 0.3f, 2, COLOR_WHEEL);

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 2.0f, w - 4.0f, h * 0.25f }, 0.2f, 4, bodyColor);
    DrawRectangleRounded((Rectangle){ x + 4.0f, y + 10.0f, w - 8.0f, 12.0f }, 0.2f, 2, COLOR_GLASS);
    DrawRectangleRounded((Rectangle){ x, y + h * 0.27f, w, h * 0.73f }, 0.1f, 2, (Color){ 200, 200, 205, 255 }); 
    DrawRectangleLinesEx((Rectangle){ x + 2.0f, y + h * 0.27f + 2.0f, w - 4.0f, h * 0.73f - 4.0f }, 1.0f, (Color){ 150, 150, 155, 255 });

    DrawRectangle((int)(x + 3.0f), (int)(y + 2.0f), 6, 4, (Color){ 255, 245, 200, 255 });
    DrawRectangle((int)(x + w - 9.0f), (int)(y + 2.0f), 6, 4, (Color){ 255, 245, 200, 255 });
    DrawRectangle((int)(x + 2.0f), (int)(y + h - 4.0f), 8, 3, (Color){ 220, 35, 35, 255 });
    DrawRectangle((int)(x + w - 10.0f), (int)(y + h - 4.0f), 8, 3, (Color){ 220, 35, 35, 255 });

    if (isIndicatorOn) {
        DrawCircleV((Vector2){ x + 3.0f, y + 4.0f }, 3.0f, GOLD);
        DrawCircleV((Vector2){ x + w - 3.0f, y + 4.0f }, 3.0f, GOLD);
    }
}

static void DrawAmbulanceShape(Rectangle rect) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 4.0f, w, h }, 0.25f, 4, (Color){ 0, 0, 0, 90 });

    float wheelW = 4.0f;
    float wheelH = 11.0f;
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + 8.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + 8.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x - wheelW + 1.0f, y + h - 18.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);
    DrawRectangleRounded((Rectangle){ x + w - 1.0f, y + h - 18.0f, wheelW, wheelH }, 0.4f, 2, COLOR_WHEEL);

    DrawRectangleRounded(rect, 0.2f, 6, RAYWHITE);
    DrawRectangle((int)(x + 1.0f), (int)(y + 12.0f), 3, (int)(h - 20.0f), (Color){ 230, 40, 40, 255 });
    DrawRectangle((int)(x + w - 4.0f), (int)(y + 12.0f), 3, (int)(h - 20.0f), (Color){ 230, 40, 40, 255 });
    DrawRectangleRounded((Rectangle){ x + 4.0f, y + 8.0f, w - 8.0f, 10.0f }, 0.3f, 2, COLOR_GLASS);

    float crossX = x + w / 2.0f;
    float crossY = y + h * 0.52f;
    DrawRectangle((int)(crossX - 7.0f), (int)(crossY - 2.0f), 14, 4, (Color){ 230, 40, 40, 255 });
    DrawRectangle((int)(crossX - 2.0f), (int)(crossY - 7.0f), 4, 14, (Color){ 230, 40, 40, 255 });

    DrawRectangle((int)(x + 3.0f), (int)(y + 1.0f), 6, 3, (Color){ 255, 245, 200, 255 });
    DrawRectangle((int)(x + w - 9.0f), (int)(y + 1.0f), 6, 3, (Color){ 255, 245, 200, 255 });

    bool toggle = ((int)(GetTime() * 16) % 2 == 0);
    Color lightLeft = toggle ? (Color){ 240, 30, 30, 255 } : WHITE;
    Color lightRight = !toggle ? (Color){ 240, 30, 30, 255 } : WHITE;

    DrawRectangle((int)(x + w / 2.0f - 10.0f), (int)(y + 4.0f), 20, 3, DARKGRAY);
    DrawRectangle((int)(x + w / 2.0f - 9.0f), (int)(y + 4.0f), 8, 3, lightLeft);
    DrawRectangle((int)(x + w / 2.0f + 1.0f), (int)(y + 4.0f), 8, 3, lightRight);
}

// --- PINK FUEL ITEM (F) ---
static void DrawFuelItem(Rectangle rect, Color color) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 3.0f, w, h }, 0.2f, 4, (Color){ 0, 0, 0, 60 });
    DrawRectangleRounded(rect, 0.25f, 4, color);

    DrawRectangle((int)(x + w / 2.0f - 4.0f), (int)(y - 3.0f), 8, 4, DARKGRAY);
    DrawRectangle((int)(x + w / 2.0f - 2.0f), (int)(y - 5.0f), 4, 3, LIGHTGRAY);
    DrawLineEx((Vector2){ x + 4.0f, y + 6.0f }, (Vector2){ x + w - 4.0f, y + h - 6.0f }, 2.0f, Fade(WHITE, 0.3f));
    DrawRectangleRounded((Rectangle){ x + 6.0f, y + h / 2.0f - 7.0f, w - 12.0f, 14.0f }, 0.3f, 2, WHITE);
    DrawCustomText("F", (Vector2){ x + w / 2.0f - 3.0f, y + h / 2.0f - 5.0f }, 10.0f, 1.0f, (Color){ 220, 40, 130, 255 });
}

// --- BLUE NITRO ITEM (N) ---
static void DrawNitroItem(Rectangle rect, Color color) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 3.0f, w, h }, 0.2f, 4, (Color){ 0, 0, 0, 60 });
    DrawRectangleRounded(rect, 0.25f, 4, color);

    DrawRectangle((int)(x + w / 2.0f - 4.0f), (int)(y - 3.0f), 8, 4, DARKGRAY);
    DrawRectangle((int)(x + w / 2.0f - 2.0f), (int)(y - 5.0f), 4, 3, LIGHTGRAY);
    DrawLineEx((Vector2){ x + 4.0f, y + 6.0f }, (Vector2){ x + w - 4.0f, y + h - 6.0f }, 2.0f, Fade(WHITE, 0.3f));
    DrawRectangleRounded((Rectangle){ x + 6.0f, y + h / 2.0f - 7.0f, w - 12.0f, 14.0f }, 0.3f, 2, WHITE);
    DrawCustomText("N", (Vector2){ x + w / 2.0f - 3.0f, y + h / 2.0f - 5.0f }, 10.0f, 1.0f, (Color){ 0, 110, 210, 255 });
}

// --- GREEN INVISIBILITY ITEM (I) ---
static void DrawInvisibleItem(Rectangle rect, Color color) {
    float x = rect.x;
    float y = rect.y;
    float w = rect.width;
    float h = rect.height;

    DrawRectangleRounded((Rectangle){ x + 2.0f, y + 3.0f, w, h }, 0.2f, 4, (Color){ 0, 0, 0, 60 });
    DrawRectangleRounded(rect, 0.25f, 4, color);

    DrawRectangle((int)(x + w / 2.0f - 4.0f), (int)(y - 3.0f), 8, 4, DARKGRAY);
    DrawRectangle((int)(x + w / 2.0f - 2.0f), (int)(y - 5.0f), 4, 3, LIGHTGRAY);
    DrawLineEx((Vector2){ x + 4.0f, y + 6.0f }, (Vector2){ x + w - 4.0f, y + h - 6.0f }, 2.0f, Fade(WHITE, 0.3f));
    DrawRectangleRounded((Rectangle){ x + 6.0f, y + h / 2.0f - 7.0f, w - 12.0f, 14.0f }, 0.3f, 2, WHITE);
    DrawCustomText("I", (Vector2){ x + w / 2.0f - 2.0f, y + h / 2.0f - 5.0f }, 10.0f, 1.0f, (Color){ 20, 130, 60, 255 });
}

void RenderGame(GameContext *game) {
    if (game->currentState == MENU) {
        DrawMenuScreen(game);
        return;
    }

    // --- PAUSE MENU INPUT LOGIC ---
    if (game->currentState == PAUSED) {
        Vector2 mousePos = GetMousePosition();

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
            pauseSelectedOption = 1 - pauseSelectedOption;
            PlayButtonHoverSound();
        }

        int panelW = 280;
        int panelH = 160;
        Rectangle pausePanel = { (SCREEN_WIDTH - panelW) / 2.0f, (SCREEN_HEIGHT - panelH) / 2.0f, (float)panelW, (float)panelH };
        Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 110.0f, pausePanel.y + 55.0f, 220.0f, 36.0f };
        Rectangle btnMenu   = { SCREEN_WIDTH / 2.0f - 110.0f, pausePanel.y + 102.0f, 220.0f, 36.0f };

        if (CheckCollisionPointRec(mousePos, btnResume)) {
            if (pauseSelectedOption != 0) PlayButtonHoverSound();
            pauseSelectedOption = 0;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                game->currentState = PLAYING;
            }
        } else if (CheckCollisionPointRec(mousePos, btnMenu)) {
            if (pauseSelectedOption != 1) PlayButtonHoverSound();
            pauseSelectedOption = 1;
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                PlayMenuSelectSound();
                StopPoliceSirenSound();
                StopAmbulanceSirenSound();
                StopBGM();
                game->currentState = MENU;
                currentMenuState = MENU_STATE_MAIN;
                PlayMenuBGM();
                return;
            }
        }

        if (IsKeyPressed(KEY_ENTER)) {
            PlayMenuSelectSound();
            if (pauseSelectedOption == 0) {
                game->currentState = PLAYING;
            } else {
                StopPoliceSirenSound();
                StopAmbulanceSirenSound();
                StopBGM();
                game->currentState = MENU;
                currentMenuState = MENU_STATE_MAIN;
                PlayMenuBGM();
                return;
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            game->currentState = PLAYING;
        }
    }

    // --- GAME OVER INPUT LOGIC ---
    if (game->currentState == GAME_OVER) {
        Vector2 mousePos = GetMousePosition();

        int panelW = 310;
        int panelH = 195;
        Rectangle goPanel = { (SCREEN_WIDTH - panelW) / 2.0f, (SCREEN_HEIGHT - panelH) / 2.0f, (float)panelW, (float)panelH };
        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 110.0f, goPanel.y + 142.0f, 220.0f, 32.0f };

        if ((CheckCollisionPointRec(mousePos, btnRestart) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) ||
            IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            PlayMenuSelectSound();
            InitGame(game);
            LoadUserDataFromCSV(game);
            game->attempts++;
            SaveUserDataToCSV(game);
            game->currentState = PLAYING;
            StopMenuBGM();
            PlayBGM();
            return;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            PlayMenuSelectSound();
            StopPoliceSirenSound();
            StopAmbulanceSirenSound();
            StopBGM();
            game->currentState = MENU;
            currentMenuState = MENU_STATE_MAIN;
            PlayMenuBGM();
            return;
        }
    }

    BeginDrawing();
    ClearBackground(COLOR_ASPHALT);

    DrawRectangle(0, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    
    DrawRectangle(45, 0, 5, SCREEN_HEIGHT, COLOR_CURB);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 5, SCREEN_HEIGHT, COLOR_CURB);

    for (int i = -1; i < SCREEN_HEIGHT / 40 + 2; i++) {
        float lineY = i * 40.0f + game->roadOffset;
        DrawRectangleRounded((Rectangle){ 148, lineY, 4, 22 }, 0.5f, 2, Fade(WHITE, 0.35f));
        DrawRectangleRounded((Rectangle){ 248, lineY, 4, 22 }, 0.5f, 2, Fade(WHITE, 0.35f));
    }

    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (game->enemies[i].active) {
            bool indicatorOn = game->enemies[i].isShifting && ((int)(GetTime() * 10) % 2 == 0);
            if (game->enemies[i].type == ENEMY_LORRY) {
                DrawLorryShape(game->enemies[i].rect, game->enemies[i].color, indicatorOn);
            } else {
                DrawCarShape(game->enemies[i].rect, game->enemies[i].color, false, indicatorOn);
            }
        }
    }

    for (int i = 0; i < MAX_FUELS; i++) {
        if (game->fuels[i].active) {
            DrawFuelItem(game->fuels[i].rect, game->fuels[i].color);
        }
    }

    for (int i = 0; i < MAX_NITROS; i++) {
        if (game->nitros[i].active) {
            DrawNitroItem(game->nitros[i].rect, game->nitros[i].color);
        }
    }

    for (int i = 0; i < MAX_INVISIBLES; i++) {
        if (game->invisibles[i].active) {
            DrawInvisibleItem(game->invisibles[i].rect, game->invisibles[i].color);
        }
    }

    if (game->police.active) {
        DrawCarShape(game->police.rect, BLACK, true, false);
    }

    if (game->ambulance.active) {
        DrawAmbulanceShape(game->ambulance.rect);
    }

    DrawPlayerCar(game);

    if (game->police.active && game->police.rect.y > SCREEN_HEIGHT) {
        if ((int)(GetTime() * 10) % 2 == 0) {
            DrawTriangle((Vector2){ SCREEN_WIDTH / 2.0f, 75.0f }, (Vector2){ SCREEN_WIDTH / 2.0f - 10.0f, 90.0f }, (Vector2){ SCREEN_WIDTH / 2.0f + 10.0f, 90.0f }, (Color){ 30, 110, 255, 255 });
            Vector2 textSz = MeasureCustomText("POLICE BEHIND!", 10.0f, 1.0f);
            DrawCustomText("POLICE BEHIND!", (Vector2){ SCREEN_WIDTH / 2.0f - textSz.x / 2.0f, 95.0f }, 10.0f, 1.0f, (Color){ 30, 110, 255, 255 });
        }
    }

    if (game->ambulance.active && game->ambulance.rect.y > SCREEN_HEIGHT) {
        if ((int)(GetTime() * 12) % 2 == 0) {
            DrawTriangle((Vector2){ SCREEN_WIDTH / 2.0f, 75.0f }, (Vector2){ SCREEN_WIDTH / 2.0f - 10.0f, 90.0f }, (Vector2){ SCREEN_WIDTH / 2.0f + 10.0f, 90.0f }, (Color){ 230, 40, 40, 255 });
            Vector2 textSz = MeasureCustomText("AMBULANCE!", 10.0f, 1.0f);
            DrawCustomText("AMBULANCE!", (Vector2){ SCREEN_WIDTH / 2.0f - textSz.x / 2.0f, 95.0f }, 10.0f, 1.0f, (Color){ 230, 40, 40, 255 });
        }
    }

    // Professional Top HUD Dashboard Panel
    DrawRectangle(0, 0, SCREEN_WIDTH, 70, COLOR_UI_PANEL);
    DrawLine(0, 70, SCREEN_WIDTH, 70, COLOR_UI_BORDER);

    DrawCustomText("SCORE", (Vector2){ 18.0f, 12.0f }, 10.0f, 1.0f, (Color){ 130, 145, 165, 255 });
    DrawCustomText(TextFormat("%06i", (int)game->floatScore), (Vector2){ 18.0f, 25.0f }, 18.0f, 1.0f, WHITE);
    DrawCustomText(TextFormat("DRIVER: %s (Att: %d)", game->playerName, game->attempts), (Vector2){ 18.0f, 48.0f }, 9.0f, 1.0f, (Color){ 140, 155, 175, 255 });
    
    // Centered Speedometer Capsule
    float speedBoxW = 90.0f;
    float speedBoxH = 22.0f;
    float speedBoxX = (SCREEN_WIDTH - speedBoxW) / 2.0f;
    float speedBoxY = 24.0f;
    Rectangle speedRect = { speedBoxX, speedBoxY, speedBoxW, speedBoxH };
    DrawRectangleRounded(speedRect, 0.4f, 2, (Color){ 22, 28, 38, 255 });
    DrawRectangleRoundedLines(speedRect, 0.4f, 2, (Color){ 50, 130, 220, 255 });
    
    const char *speedStr = TextFormat("%03i KM/H", (int)(game->gameSpeed / 2.0f));
    Vector2 speedTextSz = MeasureCustomText(speedStr, 10.0f, 1.0f);
    DrawCustomText(speedStr, (Vector2){ speedBoxX + (speedBoxW - speedTextSz.x) / 2.0f, speedBoxY + (speedBoxH - speedTextSz.y) / 2.0f }, 10.0f, 1.0f, (Color){ 0, 200, 255, 255 });

    float maxFuelWidth = 115.0f;
    float currentFuelWidth = (game->fuel / 100.0f) * maxFuelWidth;
    if (currentFuelWidth < 0.0f) currentFuelWidth = 0.0f;

    Color fuelColor = game->fuel > 50.0f ? (Color){ 46, 204, 113, 255 } : 
                     (game->fuel > 20.0f ? (Color){ 241, 196, 15, 255 } : (Color){ 231, 76, 60, 255 });

    DrawCustomText("FUEL", (Vector2){ (float)(SCREEN_WIDTH - 140), 12.0f }, 10.0f, 1.0f, (Color){ 130, 145, 165, 255 });
    
    DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 140, 27, maxFuelWidth, 16 }, 0.4f, 4, (Color){ 22, 25, 32, 255 });
    if (currentFuelWidth > 0.0f) {
        DrawRectangleRounded((Rectangle){ SCREEN_WIDTH - 140, 27, currentFuelWidth, 16 }, 0.4f, 4, fuelColor);
    }
    DrawRectangleRoundedLines((Rectangle){ SCREEN_WIDTH - 140, 27, maxFuelWidth, 16 }, 0.4f, 4, COLOR_UI_BORDER);
    
    const char *fuelStr = TextFormat("%d%%", (int)game->fuel);
    Vector2 fuelTextSz = MeasureCustomText(fuelStr, 10.0f, 1.0f);
    DrawCustomText(fuelStr, (Vector2){ SCREEN_WIDTH - 140.0f + maxFuelWidth - fuelTextSz.x - 4.0f, 46.0f }, 10.0f, 1.0f, fuelColor);

    // --- NITRO BOOST & INVISIBILITY REMAINING TIME HUD ---
    if (game->nitroTimer > 0.0f) {
        float nitroBarW = 160.0f;
        float nitroBarH = 26.0f;
        Rectangle nitroBox = { (SCREEN_WIDTH - nitroBarW) / 2.0f, 76.0f, nitroBarW, nitroBarH };
        DrawRectangleRounded(nitroBox, 0.4f, 4, (Color){ 10, 25, 45, 235 });
        DrawRectangleRoundedLines(nitroBox, 0.4f, 4, (Color){ 0, 180, 255, 255 });

        const char *nitroStr = TextFormat("NITRO BOOST: %.1f s", game->nitroTimer);
        Vector2 nitroSz = MeasureCustomText(nitroStr, 11.0f, 1.0f);
        DrawCustomText(nitroStr, (Vector2){ nitroBox.x + (nitroBarW - nitroSz.x) / 2.0f, nitroBox.y + (nitroBarH - nitroSz.y) / 2.0f }, 11.0f, 1.0f, (Color){ 0, 220, 255, 255 });
    }

    if (game->invisibleTimer > 0.0f) {
        float invBarW = 160.0f;
        float invBarH = 26.0f;
        float invY = (game->nitroTimer > 0.0f) ? 107.0f : 76.0f;
        Rectangle invBox = { (SCREEN_WIDTH - invBarW) / 2.0f, invY, invBarW, invBarH };
        DrawRectangleRounded(invBox, 0.4f, 4, (Color){ 12, 35, 20, 235 });
        DrawRectangleRoundedLines(invBox, 0.4f, 4, (Color){ 46, 204, 113, 255 });

        const char *invStr = TextFormat("INVISIBLE: %.1f s", game->invisibleTimer);
        Vector2 invSz = MeasureCustomText(invStr, 11.0f, 1.0f);
        DrawCustomText(invStr, (Vector2){ invBox.x + (invBarW - invSz.x) / 2.0f, invBox.y + (invBarH - invSz.y) / 2.0f }, 11.0f, 1.0f, (Color){ 46, 204, 113, 255 });
    }

    // Pause Menu Overlay Modal
    if (game->currentState == PAUSED) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 8, 10, 14, 210 });

        int panelW = 280;
        int panelH = 160;
        Rectangle pausePanel = { (SCREEN_WIDTH - panelW) / 2.0f, (SCREEN_HEIGHT - panelH) / 2.0f, (float)panelW, (float)panelH };
        DrawRectangleRounded(pausePanel, 0.12f, 6, (Color){ 18, 22, 30, 240 });
        DrawRectangleRoundedLines(pausePanel, 0.12f, 6, (Color){ 45, 65, 95, 255 });

        const char *pauseTitle = "GAME PAUSED";
        Vector2 pauseTitleSz = MeasureCustomText(pauseTitle, 14.0f, 1.0f);
        DrawCustomText(pauseTitle, (Vector2){ pausePanel.x + (panelW - pauseTitleSz.x) / 2.0f, pausePanel.y + 20.0f }, 14.0f, 1.0f, (Color){ 0, 200, 255, 255 });
        DrawLine((int)pausePanel.x + 20, (int)pausePanel.y + 42, (int)pausePanel.x + panelW - 20, (int)pausePanel.y + 42, Fade(WHITE, 0.15f));

        Rectangle btnResume = { SCREEN_WIDTH / 2.0f - 110.0f, pausePanel.y + 55.0f, 220.0f, 36.0f };
        Rectangle btnMenu   = { SCREEN_WIDTH / 2.0f - 110.0f, pausePanel.y + 102.0f, 220.0f, 36.0f };

        DrawCapsuleButton(btnResume, "RESUME",        pauseSelectedOption == 0);
        DrawCapsuleButton(btnMenu,   "EXIT TO MENU",  pauseSelectedOption == 1);
    }

    // Game Over Overlay Modal
    if (game->currentState == GAME_OVER) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){ 10, 5, 8, 220 });

        int panelW = 310;
        int panelH = 195;
        Rectangle goPanel = { (SCREEN_WIDTH - panelW) / 2.0f, (SCREEN_HEIGHT - panelH) / 2.0f, (float)panelW, (float)panelH };
        DrawRectangleRounded(goPanel, 0.12f, 6, (Color){ 20, 15, 22, 245 });
        DrawRectangleRoundedLines(goPanel, 0.12f, 6, (Color){ 231, 76, 60, 255 });

        const char *goTitle = "RACE TERMINATED";
        Vector2 goTitleSz = MeasureCustomText(goTitle, 16.0f, 1.0f);
        DrawCustomText(goTitle, (Vector2){ goPanel.x + (panelW - goTitleSz.x) / 2.0f, goPanel.y + 18.0f }, 16.0f, 1.0f, (Color){ 231, 76, 60, 255 });
        DrawLine((int)goPanel.x + 20, (int)goPanel.y + 42, (int)goPanel.x + panelW - 20, (int)goPanel.y + 42, Fade(WHITE, 0.15f));

        DrawCustomText("FINAL SCORE:", (Vector2){ goPanel.x + 30.0f, goPanel.y + 55.0f }, 11.0f, 1.0f, (Color){ 160, 170, 185, 255 });
        DrawCustomText(TextFormat("%06i", (int)game->floatScore), (Vector2){ goPanel.x + 190.0f, goPanel.y + 55.0f }, 12.0f, 1.0f, WHITE);

        DrawCustomText("PERSONAL BEST:", (Vector2){ goPanel.x + 30.0f, goPanel.y + 80.0f }, 11.0f, 1.0f, (Color){ 160, 170, 185, 255 });
        DrawCustomText(TextFormat("%06i", game->highScore), (Vector2){ goPanel.x + 190.0f, goPanel.y + 80.0f }, 12.0f, 1.0f, GOLD);

        DrawCustomText("ATTEMPTS:", (Vector2){ goPanel.x + 30.0f, goPanel.y + 105.0f }, 11.0f, 1.0f, (Color){ 160, 170, 185, 255 });
        DrawCustomText(TextFormat("%d", game->attempts), (Vector2){ goPanel.x + 190.0f, goPanel.y + 105.0f }, 12.0f, 1.0f, WHITE);

        Rectangle btnRestart = { SCREEN_WIDTH / 2.0f - 110.0f, goPanel.y + 142.0f, 220.0f, 32.0f };
        DrawCapsuleButton(btnRestart, "PLAY AGAIN", true);
    }

    EndDrawing();
}