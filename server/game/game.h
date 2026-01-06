
#ifndef SNAKEGAME_GAME_H
#define SNAKEGAME_GAME_H

#include "snake.h"
#include "../../common/game_protocol.h"

#define MAX_SNAKES 5

typedef struct Game Game;
typedef struct GameState GameState;

typedef void (*SnakeCollisionCallback)(Game *game, int snake_idx);

// Lifecycle
Game* game_create(int width, int height);
void game_destroy(Game* g);

// Game logic
void game_update(Game* g, SnakeCollisionCallback on_collision); // pohyb hadov, kolízie, jedlo
Snake* game_add_snake(Game* g, int start_x, int start_y);
Snake* game_reset_snake(Game *g, int idx);
void game_snake_set_direction(Game *g, int snake_idx, int direction);
void game_set_snake_dead(Game* g, int snake_idx);
void game_set_snake_alive(Game* g, int snake_idx);
void game_player_reconnect(Game* g, int idx);
Snake* game_spawn_player(Game* g, int player_idx);

// Getters
int game_get_width(const Game *g);
int game_get_height(const Game *g);
int game_is_over(const Game* g);

// Gamestate - read-only
GameState* game_get_state(Game* g);
void game_free_state(GameState* s);

// Gamestate getters
int gamestate_get_num_snakes(const GameState* s);
int gamestate_get_snake_length(const GameState* s, int snake);
int gamestate_get_snake_score(const GameState* s, int snake);
int gamestate_get_snake_segment_x(const GameState* s, int snake, int segment);
int gamestate_get_snake_segment_y(const GameState* s, int snake, int segment);
int gamestate_get_fruit_x(const GameState* s);
int gamestate_get_fruit_y(const GameState* s);
int gamestate_is_game_over(const GameState *s);
int gamestate_is_snake_alive(const GameState *state, int snake);

// Serialization
void gamestate_serialize(const GameState* state, SerializedGameState* out);

#endif