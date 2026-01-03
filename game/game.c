
#include "game.h"
#include "../common/game_protocol.h"

#include <stdlib.h>
#include <string.h>

#include "fruit.h"

struct GameState {
    int num_snakes;
    int snake_lengths[MAX_SNAKES];
    int snake_segments_x[MAX_SNAKES][MAX_SNAKE_LENGTH];
    int snake_segments_y[MAX_SNAKES][MAX_SNAKE_LENGTH];
    int snake_scores[MAX_SNAKES];
    bool snake_alive[MAX_SNAKES];
    int fruit_x;
    int fruit_y;
    int game_over;
};

struct Game {
    int width, height;
    Snake* snakes[MAX_SNAKES];
    int num_snakes;
    int snake_alive[MAX_SNAKES];
    Fruit *fruit;
    // int is_game_over;
};

Game* game_create(int width, int height) {
    Game *g = malloc(sizeof(Game));
    g->width = width;
    g->height = height;
    g->num_snakes = 0;
    memset(g->snake_alive, 1, sizeof(g->snake_alive));

    g->fruit = create_fruit(width, height);
    // g->is_game_over = 0;

    memset(g->snakes, 0, sizeof(g->snakes));
    return g;
}

void game_destroy(Game* g) {
    if (!g) return;
    for (int i = 0; i < g->num_snakes; i++) {
        snake_destroy(g->snakes[i]);
    }
    destroy_fruit(g->fruit);
    free(g);
}

Snake* game_add_snake(Game* g, int start_x, int start_y) {
    if (g->num_snakes >= MAX_SNAKES) return NULL;

    int idx = g->num_snakes;
    Snake *s = snake_create(start_x, start_y);

    g->snakes[idx] = s;
    g->snake_alive[idx] = 1;
    g->num_snakes++;
    return s;
}

void game_update(Game* g) {
    if (!g) return;

    for (int i = 0; i < g->num_snakes; i++) {
        if (!g->snake_alive[i]) continue;

        Snake* s = g->snakes[i];
        snake_move(s, g->width, g->height);

        if (snake_check_self_collision(s)) {
            g->snake_alive[i] = 0;
            continue;
        }

        if (snake_get_x(s) == fruit_get_x(g->fruit) &&
            snake_get_y(s) == fruit_get_y(g->fruit)) {
            snake_grow(s);
            fruit_new_coordinates(g->fruit, g->width, g->height);
            }
    }
}

int game_get_width(Game *g) {
    if (!g) return 0;
    return g->width;
}

int game_get_heigth(Game *g) {
    if (!g) return 0;
    return g->height;
}


GameState* game_get_state(Game* g) {
    if (!g) return NULL;

    GameState* state = malloc(sizeof(GameState));
    memset(state, 0, sizeof(GameState));

    state->num_snakes = g->num_snakes;

    for (int i = 0; i < g->num_snakes; i++) {
        state->snake_alive[i] = g->snake_alive[i];
    }

    for (int i = 0; i < g->num_snakes; i++) {
        Snake* s = g->snakes[i];
        int len = snake_get_length(s);
        state->snake_lengths[i] = len;
        state->snake_scores[i] = snake_get_score(s);

        for (int j = 0; j < len; j++) {
            state->snake_segments_x[i][j] = snake_get_segment_x(s, j);
            state->snake_segments_y[i][j] = snake_get_segment_y(s, j);
        }
    }

    state->fruit_x = fruit_get_x(g->fruit);
    state->fruit_y = fruit_get_y(g->fruit);

    return state;
}

void game_free_state(GameState* s) {
    free(s);
}

int gamestate_get_num_snakes(GameState* s) {
    return s ? s->num_snakes : 0;
}

int gamestate_get_snake_length(GameState* s, int snake) {
    return (s && snake >= 0 && snake < s->num_snakes) ? s->snake_lengths[snake] : 0;
}

int gamestate_get_snake_score(GameState* s, int snake) {
    return (s && snake >= 0 && snake < s->num_snakes) ? s->snake_scores[snake] : 0;
}

int gamestate_get_snake_segment_x(GameState* s, int snake, int segment) {
    if (!s || snake < 0 || snake >= s->num_snakes) return 0;
    if (segment < 0 || segment >= s->snake_lengths[snake]) return 0;
    return s->snake_segments_x[snake][segment];
}

int gamestate_get_snake_segment_y(GameState* s, int snake, int segment) {
    if (!s || snake < 0 || snake >= s->num_snakes) return 0;
    if (segment < 0 || segment >= s->snake_lengths[snake]) return 0;
    return s->snake_segments_y[snake][segment];
}

int gamestate_get_fruit_x(GameState* s) {
    return s ? s->fruit_x : 0;
}

int gamestate_get_fruit_y(GameState* s) {
    return s ? s->fruit_y : 0;
}

int gamestate_is_game_over(GameState *s) {
    if (!s) return 0;
    return s->game_over;
}

void gamestate_set_game_over(GameState *s, int game_over) {
    if (!s) return;
    s->game_over = game_over;
}

void gamestate_set_fruit(GameState *s, int fruit_x, int fruit_y) {
    if (!s) return;
    s->fruit_x = fruit_x;
    s->fruit_y = fruit_y;
}

void gamestate_set_snake_score(GameState* state, int snake_idx, int score) {
    if (!state) return;
    if (snake_idx < 0 || snake_idx >= state->num_snakes) return;

    state->snake_scores[snake_idx] = score;
}

void gamestate_set_snake_segment(GameState* state, int snake_idx, int segment_idx, int x, int y) {
    if (!state) return;
    if (snake_idx < 0 || snake_idx >= state->num_snakes) return;
    if (segment_idx < 0 || segment_idx >= state->snake_lengths[snake_idx]) return;

    state->snake_segments_x[snake_idx][segment_idx] = x;
    state->snake_segments_y[snake_idx][segment_idx] = y;
}

int gamestate_is_snake_alive(GameState *state, int snake) {
    if (!state) return 0;
    if (snake < 0 || snake >= state->num_snakes) return 0;
    return state->snake_alive[snake];
}


void gamestate_serialize(GameState* state, SerializedGameState* out) {
    // out->num_snakes = gamestate_get_num_snakes(state);
    // for (int i = 0; i < out->num_snakes; i++) {
    //     out->snake_lengths[i] = gamestate_get_snake_length(state, i);
    //     for (int j = 0; j < out->snake_lengths[i]; j++) {
    //         out->snake_x[i][j] = gamestate_get_snake_segment_x(state, i, j);
    //         out->snake_y[i][j] = gamestate_get_snake_segment_y(state, i, j);
    //     }
    //     out->snake_scores[i] = gamestate_get_snake_score(state, i);
    // }
    // out->fruit_x = gamestate_get_fruit_x(state);
    // out->fruit_y = gamestate_get_fruit_y(state);
    // out->game_over = gamestate_is_game_over(state);
    out->num_snakes = state->num_snakes;

    for (int i = 0; i < out->num_snakes; i++) {
        out->snake_lengths[i] = state->snake_lengths[i];
        out->snake_scores[i] = state->snake_scores[i];
        out->snake_alive[i]  = state->snake_alive[i]; // NEW

        for (int j = 0; j < out->snake_lengths[i]; j++) {
            out->snake_x[i][j] = state->snake_segments_x[i][j];
            out->snake_y[i][j] = state->snake_segments_y[i][j];
        }
    }

    out->fruit_x = state->fruit_x;
    out->fruit_y = state->fruit_y;
}

void gamestate_deserialize(GameState* state, const SerializedGameState* in) {
    // update state via setters, not direct struct access
    state->num_snakes = in->num_snakes;

    for (int i = 0; i < in->num_snakes; i++) {
        state->snake_alive[i] = in->snake_alive[i];

        for (int j = 0; j < in->snake_lengths[i]; j++) {
            gamestate_set_snake_segment(state, i, j,
                                        in->snake_x[i][j],
                                        in->snake_y[i][j]);
        }

        gamestate_set_snake_score(state, i, in->snake_scores[i]);
    }

    gamestate_set_fruit(state, in->fruit_x, in->fruit_y);
}