
#ifndef SNAKEGAMEREFACTORED_GAME_H
#define SNAKEGAMEREFACTORED_GAME_H

#include "snake.h"

#define MAX_SNAKES 5

typedef struct Game Game;
typedef struct GameState GameState;

Game* game_create(int width, int height);
void game_destroy(Game* g);

Snake* game_add_snake(Game* g, int start_x, int start_y);
void game_update(Game* g); // pohyb hadov, kolízie, jedlo

int game_get_width(Game *g);
int game_get_heigth(Game *g);
int game_is_over(const Game* g);

GameState* game_get_state(Game* g);
void game_free_state(GameState* s);

// Accessors for rendering:
int gamestate_get_num_snakes(GameState* s);
int gamestate_get_snake_length(GameState* s, int snake);
int gamestate_get_snake_score(GameState* s, int snake);
int gamestate_get_snake_segment_x(GameState* s, int snake, int segment);
int gamestate_get_snake_segment_y(GameState* s, int snake, int segment);
int gamestate_get_fruit_x(GameState* s);
int gamestate_get_fruit_y(GameState* s);

int gamestate_is_game_over(GameState *s);
int gamestate_is_snake_alive(GameState *state, int snake);
void gamestate_set_game_over(GameState *s, int game_over);
void gamestate_set_fruit(GameState *s, int fruit_x, int fruit_y);
void gamestate_set_snake_score(GameState* state, int snake_idx, int score);
void gamestate_set_snake_segment(GameState* state, int snake_idx, int segment_idx, int x, int y);


#endif //SNAKEGAMEREFACTORED_GAME_H