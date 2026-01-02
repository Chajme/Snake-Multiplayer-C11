#include "game_protocol.h"
#include "../game/game.h" // access to getters/setters of GameState

void gamestate_serialize(const GameState* state, SerializedGameState* out) {
    out->num_snakes = gamestate_get_num_snakes(state);
    for (int i = 0; i < out->num_snakes; i++) {
        out->snake_lengths[i] = gamestate_get_snake_length(state, i);
        for (int j = 0; j < out->snake_lengths[i]; j++) {
            out->snake_x[i][j] = gamestate_get_snake_segment_x(state, i, j);
            out->snake_y[i][j] = gamestate_get_snake_segment_y(state, i, j);
        }
        out->snake_scores[i] = gamestate_get_snake_score(state, i);
    }
    out->fruit_x = gamestate_get_fruit_x(state);
    out->fruit_y = gamestate_get_fruit_y(state);
    out->game_over = gamestate_is_game_over(state);
}

void gamestate_deserialize(GameState* state, const SerializedGameState* in) {
    for (int i = 0; i < in->num_snakes; i++) {
        for (int j = 0; j < in->snake_lengths[i]; j++) {
            gamestate_set_snake_segment(state, i, j, in->snake_x[i][j], in->snake_y[i][j]);
        }
        gamestate_set_snake_score(state, i, in->snake_scores[i]);
    }
    gamestate_set_fruit(state, in->fruit_x, in->fruit_y);
    gamestate_set_game_over(state, in->game_over);
}
