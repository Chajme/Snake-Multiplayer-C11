#include "game.h"
#include "../../common/game_protocol.h"
#include "../../util/vector.h"

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

    Vector* snakes;
    Fruit* fruit;
    GameState state;
};

static void snake_ptr_destroy(void* elem) {
    Snake* s = *(Snake**)elem;
    if (s) snake_destroy(s);
}

static void game_sync_state(Game* g) {
    GameState* s = &g->state;

    s->num_snakes = (int)vector_get_size(g->snakes);
    s->game_over = false;

    for (size_t i = 0; i < vector_get_size(g->snakes); i++) {
        Snake** sptr = vector_get(g->snakes, i);
        Snake* snake = *sptr;

        if (!snake) {
            s->snake_alive[i] = 0;
            continue;
        }

        s->snake_alive[i] = snake_is_alive(snake);

        int len = snake_get_length(snake);
        s->snake_lengths[i] = len;
        s->snake_scores[i] = snake_get_score(snake);

        for (int j = 0; j < len; j++) {
            s->snake_segments_x[i][j] = snake_get_segment_x(snake, j);
            s->snake_segments_y[i][j] = snake_get_segment_y(snake, j);
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
    Game* g = malloc(sizeof(Game));
    if (!g) return NULL;

    g->width = width;
    g->height = height;
    g->snakes = vector_new(sizeof(Snake*), NULL, snake_ptr_destroy);
    g->fruit = fruit_create(width, height);

    game_sync_state(g);
    return g;
}

void game_destroy(Game* g) {
    if (!g) return;

    vector_free(g->snakes);
    fruit_destroy(g->fruit);
    free(g);
}

void game_player_reconnect(Game* g, int idx) {
    if (!g) return;
    if (idx < 0 || idx >= MAX_SNAKES) return;

    Snake** sptr = vector_get(g->snakes, idx);
    if (!sptr) return;

    if (*sptr) {
        snake_destroy(*sptr);
        *sptr = NULL;
    }

    *sptr = snake_create(2 + idx * 3, 2 + idx * 2);
    snake_set_alive(*sptr, true);

    g->state.game_over = 0;
}

Snake* game_spawn_player(Game* g, int player_idx) {
    if (!g || player_idx < 0 || player_idx >= MAX_SNAKES) return NULL;

    while ((int)vector_get_size(g->snakes) <= player_idx) {
        Snake* null_snake = NULL;
        vector_push_back(g->snakes, &null_snake);
    }

    Snake** sptr = vector_get(g->snakes, player_idx);
    if (*sptr) {
        snake_destroy(*sptr);
    }

    *sptr = snake_create(2 + player_idx * 3, 2 + player_idx * 2);
    snake_set_alive(*sptr, true);

    g->state.game_over = 0;

    return *sptr;
}

Snake* game_add_snake(Game* g, int start_x, int start_y) {
    if (!g) return NULL;

    Snake* s = snake_create(start_x, start_y);
    snake_set_alive(s, true);
    vector_push_back(g->snakes, &s);
    return s;
}

Snake* game_reset_snake(Game* g, int idx) {
    if (!g) return NULL;
    if (idx < 0 || idx >= (int)vector_get_size(g->snakes)) return NULL;

    Snake** sptr = vector_get(g->snakes, idx);
    if (!sptr) return NULL;

    if (*sptr) snake_destroy(*sptr);

    *sptr = snake_create(2 + idx * 3, 2 + idx * 2);
    snake_set_alive(*sptr, true);  // <- correct
    return *sptr;
}

void game_set_snake_dead(Game* g, int idx) {
    if (!g || idx < 0 || idx >= (int)vector_get_size(g->snakes)) return;

    Snake** sptr = vector_get(g->snakes, idx);
    if (!sptr || !*sptr) return;

    snake_set_alive(*sptr, false);
}

void game_set_snake_alive(Game* g, int idx) {
    if (!g || idx < 0 || idx >= (int)vector_get_size(g->snakes)) return;

    Snake** sptr = vector_get(g->snakes, idx);
    if (!sptr || !*sptr) return;

    snake_set_alive(*sptr, true);
}

void game_snake_set_direction(Game* g, int snake_idx, int direction) {
    if (!g || snake_idx < 0 || snake_idx >= (int)vector_get_size(g->snakes)) return;

    Snake** sptr = vector_get(g->snakes, snake_idx);
    if (!sptr || !*sptr || !snake_is_alive(*sptr)) return;

    snake_set_direction(*sptr, direction);
}

void game_update(Game* g, SnakeCollisionCallback on_collision) {
    if (!g) return;

    size_t count = vector_get_size(g->snakes);

    for (size_t i = 0; i < count; i++) {
        Snake* s = *(Snake**)vector_get(g->snakes, i);
        if (!s || !snake_is_alive(s)) continue;

        snake_move(s, g->width, g->height);

        if (snake_check_self_collision(s)) {
            if (on_collision) on_collision(g, (int)i);
            continue;
        }

        for (size_t j = 0; j < count; j++) {
            if (i == j) continue;

            Snake* other = *(Snake**)vector_get(g->snakes, j);
            if (!other || !snake_is_alive(other)) continue;

            if (game_check_player_collision(s, other)) {
                if (on_collision) on_collision(g, (int)i);
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

int game_get_height(const Game *g) {
    if (!g) return 0;
    return g->height;
}


GameState* game_get_state(Game* g) {
    if (!g) return NULL;

    GameState* copy = malloc(sizeof(GameState));
    if (!copy) return NULL;

    *copy = g->state; // struct copy
    return copy;
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