#include "snake.h"

#include <stdlib.h>

struct Position {
    int x, y;
};

struct Snake {
    Position segments[MAX_SNAKE_LENGTH];
    int length;
    int direction;
    int score;
};

Snake* snake_create(int startX, int startY) {
    Snake *s = malloc(sizeof(Snake));
    s->segments[0].x = startX;
    s->segments[0].y = startY;
    s->length = 3;
    s->direction = 1;   // napr. doprava
    s->score = 0;
    return s;
}

void snake_destroy(Snake* s) {
    if (!s) return;
    free(s);
}

void snake_move(Snake* s, int width, int height) {
    if (!s) return;

    // posun segmentov od tail ku hlave
    for (int i = s->length - 1; i > 0; i--) {
        s->segments[i] = s->segments[i-1];
    }

    Position* head = &s->segments[0];
    switch(s->direction) {
        case 0: head->y--; break; // hore
        case 1: head->x++; break; // pravo
        case 2: head->y++; break; // dole
        case 3: head->x--; break; // ľavo
    }

    // wrap-around
    if (head->x < 0) head->x = width - 1;
    if (head->x >= width) head->x = 0;
    if (head->y < 0) head->y = height - 1;
    if (head->y >= height) head->y = 0;
}

void snake_grow(Snake* s) {
    if (s->length < MAX_SNAKE_LENGTH) {
        s->segments[s->length] = s->segments[s->length - 1]; // pridáme segment na koniec
        s->length++;
        s->score++;
    }
}

void snake_set_direction(Snake* s, int direction) {
    if (!s) return;

    // Prevent 180° turn
    if ((s->direction == 0 && direction == 2) ||
        (s->direction == 2 && direction == 0) ||
        (s->direction == 1 && direction == 3) ||
        (s->direction == 3 && direction == 1)) {
        return; // ignore
        }

    s->direction = direction;
}

int snake_check_self_collision(const Snake* s) {
    if (!s || s->length < 4) return 0;

    int head_x = s->segments[0].x;
    int head_y = s->segments[0].y;

    for (int i = 1; i < s->length; i++) {
        if (s->segments[i].x == head_x &&
            s->segments[i].y == head_y) {
            return 1;
            }
    }
    return 0;
}


void snake_set_position(Snake* s, int x, int y) {
    if (!s) return;
    s->segments[0].x = x;
    s->segments[0].y = y;
}

int snake_get_x(Snake* s) {
    if (!s) return 0;
    return s->segments[0].x;
}

int snake_get_y(Snake* s) {
    if (!s) return 0;
    return s->segments[0].y;
}

int snake_get_length(Snake *s) {
    if (!s) return 0;
    return s->length;
}

int snake_get_score(Snake* s) {
    if (!s) return 0;
    return s->score;
}

int snake_get_segment_x(Snake *s, int segment) {
    if (!s) return 0;
    return s->segments[segment].x;
}

int snake_get_segment_y(Snake *s, int segment) {
    if (!s) return 0;
    return s->segments[segment].y;
}