//
// Created by erikm on 27. 12. 2025.
//

#ifndef UDSP_SEMESTRALKA_3_GAME_H
#define UDSP_SEMESTRALKA_3_GAME_H

#include <SDL2/SDL.h>
#include "../common/protocol.h"

void ResetGame(PlayerState *player);
void UpdateGame(PlayerState *player);
void DrawGame(SDL_Renderer *r, int playerId, GameState *state);

#endif //UDSP_SEMESTRALKA_3_GAME_H