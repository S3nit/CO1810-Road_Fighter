#include "raylib.h"
#include "game.h"
#include "ui.h"
#include "audiomanager.h"

int main(void) {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Road Fighter");
    SetTargetFPS(60);

    InitAudioManager();

    GameContext game;
    InitGame(&game);

    while (!WindowShouldClose() && !game.exitRequested) {
        float dt = GetFrameTime();

        // Update audio every frame so menu BGM streams properly
        UpdateAudio(&game, dt);

        if (game.currentState == MENU) {
            UpdateMenuLogic(&game);
            DrawMenuScreen(&game);
        } else {
            UpdateGame(&game, dt);
            RenderGame(&game);
        }
    }

    CloseAudioManager();
    CloseWindow();
    return 0;
}