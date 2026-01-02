#ifndef SNAKEGAMEREFACTORED_GAME_RENDER_H
#define SNAKEGAMEREFACTORED_GAME_RENDER_H

#include <SDL_pixels.h>

#include "../game/game.h"
#include "../common/game_protocol.h"

typedef struct GameRenderer GameRenderer;  // forward declaration

int renderer_init(GameRenderer** gr, const char* title, int width, int height, int cell_size);
void renderer_destroy(GameRenderer* gr);

void rendered_draw_grid(GameRenderer *gr, int width, int height);
void renderer_draw_game(GameRenderer* gr, GameState* state);
void renderer_draw_game_over_overlay(GameRenderer *gr, int score);
void renderer_draw_text(GameRenderer* gr, const char* text, int x, int y, SDL_Color color);
void renderer_draw_serialized(GameRenderer* gr, const SerializedGameState* s);

#endif //SNAKEGAMEREFACTORED_GAME_RENDER_H