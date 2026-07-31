#ifndef UI_H
#define UI_H

#include "game.h"

void RenderGame(GameContext *game);
void UpdateMenuLogic(GameContext *game);
void DrawMenuScreen(const GameContext *game);
void DrawGameplay(GameContext *game);

#endif // UI_H
