#ifndef UDSP_SEMESTRALKA_3_GAME_RENDER_H
#define UDSP_SEMESTRALKA_3_GAME_RENDER_H

#include "../game/game.h"
#include <SDL2/SDL.h>

void DrawGame(SDL_Renderer *r, int playerId, GameState *state);

#endif //UDSP_SEMESTRALKA_3_GAME_RENDER_H