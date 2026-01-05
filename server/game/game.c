
#include "game.h"
#include "../../common/game_protocol.h"

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
    int width;
    int height;
    Snake* snakes[MAX_SNAKES];
    int snake_alive[MAX_SNAKES];
    int num_snakes;

    Fruit *fruit;

    GameState state;
};

static void game_sync_state(Game* g) {
    GameState* s = &g->state;

    s->num_snakes = g->num_snakes;
    s->game_over = false;

    for (int i = 0; i < g->num_snakes; i++) {
        s->snake_alive[i] = g->snake_alive[i];

        if (!g->snakes[i]) continue;

        int len = snake_get_length(g->snakes[i]);
        s->snake_lengths[i] = len;
        s->snake_scores[i] = snake_get_score(g->snakes[i]);

        for (int j = 0; j < len; j++) {
            s->snake_segments_x[i][j] = snake_get_segment_x(g->snakes[i], j);
            s->snake_segments_y[i][j] = snake_get_segment_y(g->snakes[i], j);
        }
    }

    s->fruit_x = fruit_get_x(g->fruit);
    s->fruit_y = fruit_get_y(g->fruit);
}

static bool game_check_player_collision(const Snake *head, const Snake *body) {
    if (!head || !body) return false;

    int hx = snake_get_x(head);
    int hy = snake_get_y(head);

    int len = snake_get_length(body);
    for (int i = 0; i < len; i++) {
        if (hx == snake_get_segment_x(body, i) &&
            hy == snake_get_segment_y(body, i)) {
            return true;
            }
    }

    return false;
}

Game* game_create(int width, int height) {
    Game *g = malloc(sizeof(Game));
    if (!g) return NULL;

    g->width = width;
    g->height = height;

    g->fruit = fruit_create(width, height);
    g->num_snakes = 0;

    for (int i = 0; i < MAX_SNAKES; i++) {
        g->snakes[i] = NULL;
        g->snake_alive[i] = false;
    }

    game_sync_state(g);
    return g;
}

void game_destroy(Game* g) {
    if (!g) return;
    for (int i = 0; i < g->num_snakes; i++) {
        if (g->snakes[i]) {
            snake_destroy(g->snakes[i]);
        }
    }

    fruit_destroy(g->fruit);
    free(g);
}

Snake* game_add_snake(Game* g, int start_x, int start_y) {
    if (!g || g->num_snakes >= MAX_SNAKES) return NULL;

    int idx = g->num_snakes;
    Snake *s = snake_create(start_x, start_y);

    g->snakes[idx] = s;
    g->snake_alive[idx] = true;
    g->num_snakes++;

    return s;
}

Snake* game_reset_snake(Game *g, int idx) {
    if (!g || idx < 0 || idx >= MAX_SNAKES) return NULL;

    if (g->snakes[idx]) {
        snake_destroy(g->snakes[idx]);
    }

    // Create fresh snake
    g->snakes[idx] = snake_create(2 + idx * 3, 2 + idx * 2);
    g->snake_alive[idx] = true;

    if (idx >= g->num_snakes) {
        g->num_snakes = idx + 1;
    }

    return g->snakes[idx];
}

void game_snake_set_direction(Game *g, int snake_idx, int direction) {
    if (!g || snake_idx < 0 || snake_idx >= MAX_SNAKES) return;
    if (!g->snake_alive[snake_idx]) return;

    snake_set_direction(g->snakes[snake_idx], direction);
}

void game_set_snake_dead(Game* g, int snake_idx) {
    if (!g || snake_idx < 0 || snake_idx >= g->num_snakes) return;
    if (!g->snakes[snake_idx]) return; // already dead

    g->snake_alive[snake_idx] = 0;
}

void game_set_snake_alive(Game* g, int snake_idx) {
    if (!g || snake_idx < 0 || snake_idx >= g->num_snakes) return;
    if (g->snakes[snake_idx]) return;

    g->snake_alive[snake_idx] = 1;
}

void game_update(Game* g) {
    if (!g) return;

    for (int i = 0; i < g->num_snakes; i++) {
        if (!g->snake_alive[i]) continue;

        Snake* s = g->snakes[i];
        snake_move(s, g->width, g->height);

        if (snake_check_self_collision(s)) {
            game_set_snake_dead(g, i);
            continue;
        }

        // Check collision with other players
        for (int j = 0; j < g->num_snakes; j++) {
            if (i == j) continue;
            if (!g->snake_alive[j]) continue;
            if (!g->snakes[j]) continue;

            if (game_check_player_collision(s, g->snakes[j])) {
                game_set_snake_dead(g, i);
                break;
            }
        }

        if (snake_get_x(s) == fruit_get_x(g->fruit) &&
            snake_get_y(s) == fruit_get_y(g->fruit)) {

            snake_grow(s);
            fruit_new_coordinates(g->fruit, g->width, g->height);
            }
    }

    game_sync_state(g);
}

int game_get_width(const Game *g) {
    if (!g) return 0;
    return g->width;
}

int game_get_heigth(const Game *g) {
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
    if (!s) return;
    free(s);
}

int gamestate_get_num_snakes(const GameState* s) {
    return s ? s->num_snakes : 0;
}

int gamestate_get_snake_length(const GameState* s, int snake) {
    return (s && snake >= 0 && snake < s->num_snakes) ? s->snake_lengths[snake] : 0;
}

int gamestate_get_snake_score(const GameState* s, int snake) {
    return (s && snake >= 0 && snake < s->num_snakes) ? s->snake_scores[snake] : 0;
}

int gamestate_get_snake_segment_x(const GameState* s, int snake, int segment) {
    if (!s || snake < 0 || snake >= s->num_snakes) return 0;
    if (segment < 0 || segment >= s->snake_lengths[snake]) return 0;
    return s->snake_segments_x[snake][segment];
}

int gamestate_get_snake_segment_y(const GameState* s, int snake, int segment) {
    if (!s || snake < 0 || snake >= s->num_snakes) return 0;
    if (segment < 0 || segment >= s->snake_lengths[snake]) return 0;
    return s->snake_segments_y[snake][segment];
}

int gamestate_get_fruit_x(const GameState* s) {
    return s ? s->fruit_x : 0;
}

int gamestate_get_fruit_y(const GameState* s) {
    return s ? s->fruit_y : 0;
}

int gamestate_is_snake_alive(const GameState *state, int snake) {
    if (!state) return 0;
    if (snake < 0 || snake >= state->num_snakes) return 0;
    return state->snake_alive[snake];
}

int gamestate_is_game_over(const GameState *s) {
    if (!s) return 0;
    return s->game_over;
}

void gamestate_serialize(const GameState* state, SerializedGameState* out) {
    if (!state || !out) return;

    memset(out, 0, sizeof(*out));

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