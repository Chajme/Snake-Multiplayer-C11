#ifndef GAME_GAME_PROTOCOL_H
#define GAME_GAME_PROTOCOL_H

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
    int fruit_x;
    int fruit_y;
    bool game_over;
} SerializedGameState;

#endif //GAME_GAME_PROTOCOL_H