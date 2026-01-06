#ifndef SNAKEGAME_RENDER_H
#define SNAKEGAME_RENDER_H

#include <SDL_pixels.h>

#include "../../common/game_protocol.h"

#define UI_PANEL_WIDTH 200

typedef struct GameRenderer GameRenderer;

int renderer_init(GameRenderer** gr, const char* title, int width, int height, int cell_size);
void renderer_destroy(GameRenderer* gr);

SDL_Color renderer_generate_snake_color(int player_id);

void rendered_draw_grid(GameRenderer *gr, int width, int height);
void renderer_draw_game_over_overlay(const GameRenderer *gr, int score);
void renderer_draw_text(const GameRenderer* gr, const char* text, int x, int y, SDL_Color color);
void renderer_draw_serialized(GameRenderer* gr, const SerializedGameState* s, int snake_id, bool client_connected);
void renderer_draw_server_shutdown_message(const GameRenderer* gr);

#endif