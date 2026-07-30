#include "screens.h"
#include <math.h>

static const Color COLOR_ASPHALT  = (Color){ 43, 45, 48, 255 };
static const Color COLOR_GRASS    = (Color){ 80, 190, 90, 255 }; // Lighter green grass land
static const Color COLOR_CURB     = (Color){ 200, 200, 200, 255 };
static const Color COLOR_UI_PANEL = (Color){ 20, 20, 25, 230 };
static const Color COLOR_PLAYER   = (Color){ 220, 50, 70, 255 };

static void DrawBackgroundRoad(float roadOffset) {
    ClearBackground(COLOR_ASPHALT);
    DrawRectangle(0, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 50, SCREEN_HEIGHT, COLOR_GRASS);
    DrawRectangle(45, 0, 5, SCREEN_HEIGHT, COLOR_CURB);
    DrawRectangle(SCREEN_WIDTH - 50, 0, 5, SCREEN_HEIGHT, COLOR_CURB);

    for (int i = -1; i < SCREEN_HEIGHT / 40 + 2; i++) {
        DrawRectangle(148, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.3f));
        DrawRectangle(248, i * 40 + roadOffset, 4, 20, Fade(WHITE, 0.3f));
    }
}

void RenderIntroScreen(float roadOffset) {
    BeginDrawing();
    DrawBackgroundRoad(roadOffset);

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.6f));

    int titleY = 180;
    const char *title = "ROAD FIGHTER";
    const char *subtitle = "TURBO EDITION";

    DrawText(title, SCREEN_WIDTH / 2 - MeasureText(title, 32) / 2 + 3, titleY + 3, 32, Fade(BLACK, 0.8f));
    DrawText(title, SCREEN_WIDTH / 2 - MeasureText(title, 32) / 2, titleY, 32, RED);

    DrawText(subtitle, SCREEN_WIDTH / 2 - MeasureText(subtitle, 16) / 2, titleY + 45, 16, GOLD);

    if ((int)(GetTime() * 3) % 2 == 0) {
        const char *prompt = "PRESS [ENTER] TO START";
        DrawText(prompt, SCREEN_WIDTH / 2 - MeasureText(prompt, 14) / 2, 420, 14, WHITE);
    }

    DrawText("v1.0 - Raylib Engine", SCREEN_WIDTH / 2 - MeasureText("v1.0 - Raylib Engine", 10) / 2, SCREEN_HEIGHT - 30, 10, DARKGRAY);

    EndDrawing();
}

void RenderMenuScreen(int selectedOption, bool showControls, float roadOffset) {
    BeginDrawing();
    DrawBackgroundRoad(roadOffset);

    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, Fade(BLACK, 0.65f));

    const char *header = "MAIN MENU";
    DrawText(header, SCREEN_WIDTH / 2 - MeasureText(header, 26) / 2, 100, 26, WHITE);
    DrawLine(80, 140, SCREEN_WIDTH - 80, 140, Fade(WHITE, 0.2f));

    const char *options[MENU_OPTION_COUNT] = { "1. DRIVE", "2. CONTROLS", "3. EXIT" };
    int startY = 220;

    for (int i = 0; i < MENU_OPTION_COUNT; i++) {
        Rectangle optionRect = { 70, startY + (i * 55), SCREEN_WIDTH - 140, 42 };
        bool isSelected = (i == selectedOption);

        if (isSelected) {
            DrawRectangleRounded(optionRect, 0.3f, 4, COLOR_PLAYER);
            DrawRectangleRoundedLines(optionRect, 0.3f, 4, WHITE);
            DrawText(options[i], optionRect.x + 30, optionRect.y + 12, 16, WHITE);
        } else {
            DrawRectangleRounded(optionRect, 0.3f, 4, COLOR_UI_PANEL);
            DrawText(options[i], optionRect.x + 30, optionRect.y + 12, 16, LIGHTGRAY);
        }
    }

    if (showControls) {
        Rectangle modal = { 40, 180, SCREEN_WIDTH - 80, 240 };
        DrawRectangleRounded(modal, 0.1f, 8, (Color){ 15, 15, 20, 245 });
        DrawRectangleRoundedLines(modal, 0.1f, 8, GOLD);

        DrawText("HOW TO PLAY", SCREEN_WIDTH / 2 - MeasureText("HOW TO PLAY", 16) / 2, 200, 16, GOLD);
        DrawText("- [UP / DOWN]  : Accelerate / Brake", 60, 245, 11, WHITE);
        DrawText("- [LEFT / RIGHT]: Steer Vehicle", 60, 275, 11, WHITE);
        DrawText("- Collect Pink Fuel Cars for Gas!", 60, 305, 11, LIME);
        DrawText("- Avoid Normal Traffic Collisions!", 60, 335, 11, RED);

        DrawText("Press [ENTER] or [ESC] to back", SCREEN_WIDTH / 2 - MeasureText("Press [ENTER] or [ESC] to back", 10) / 2, 390, 10, GRAY);
    }

    EndDrawing();
}