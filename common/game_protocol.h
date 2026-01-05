#ifndef SNAKEGAME_PROTOCOL_H
#define SNAKEGAME_PROTOCOL_H

#pragma once
#include <stdbool.h>

#define MAX_SNAKES 5
#define MAX_LENGTH 64

typedef struct {
    int num_snakes;
    int snake_lengths[MAX_SNAKES];
    int snake_x[MAX_SNAKES][MAX_LENGTH];
    int snake_y[MAX_SNAKES][MAX_LENGTH];
    int snake_scores[MAX_SNAKES];
    bool snake_alive[MAX_SNAKES];
    int fruit_x;
    int fruit_y;
    // bool game_over;
} SerializedGameState;

typedef struct {
    int player_id;
} AssignPlayerMsg;

#endif