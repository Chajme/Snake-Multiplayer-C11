#include "snake.h"

#include <stdbool.h>

#include "../../util/vector.h"

#include <stdlib.h>

struct Segment {
    int x, y;
};

struct Snake {
    Vector *segments;
    int direction;
    int score;
    bool alive;
};

Snake* snake_create(int startX, int startY) {
    Snake *s = malloc(sizeof(Snake));
    if (!s) return NULL;

    s->segments = vector_new(sizeof(Segment), NULL, NULL); // allocate the vector!

    Segment head = { startX, startY };

    vector_push_back(s->segments, &head);

    for (int i = 1; i < 3; i++) {
        vector_push_back(s->segments, &head);
    }

    s->direction = 1;
    s->score = 0;
    s->alive = true;
    return s;
}

void snake_destroy(Snake* s) {
    if (!s) return;
    vector_free(s->segments);
    free(s);
}

void snake_move(Snake* s, int width, int height) {
    if (!s) return;

    size_t len = vector_get_size(s->segments);

    for (size_t i = len - 1; i > 0; i--) {
        Segment *curr = vector_get(s->segments, i);
        Segment *prev = vector_get(s->segments, i - 1);
        *curr = *prev;
    }

    Segment *head = vector_get(s->segments, 0);

    switch (s->direction) {
        case 0: head->y--; break;
        case 1: head->x++; break;
        case 2: head->y++; break;
        case 3: head->x--; break;
        default: ;
    }

    if (head->x < 0) head->x = width - 1;
    if (head->x >= width) head->x = 0;
    if (head->y < 0) head->y = height - 1;
    if (head->y >= height) head->y = 0;

}

void snake_grow(Snake* s) {
    if (!s) return;

    Segment *tail = vector_get(s->segments, vector_get_size(s->segments) - 1);
    vector_push_back(s->segments, tail);

    s->score++;
}

void snake_set_direction(Snake* s, int direction) {
    if (!s) return;

    if ((s->direction == 0 && direction == 2) ||
        (s->direction == 2 && direction == 0) ||
        (s->direction == 1 && direction == 3) ||
        (s->direction == 3 && direction == 1)) {
        return;
        }

    s->direction = direction;
}

int snake_check_self_collision(const Snake* s) {
    if (!s || vector_get_size(s->segments) < 4) return 0;

    Segment *head = vector_get(s->segments, 0);

    for (size_t i = 1; i < vector_get_size(s->segments); i++) {
        Segment *seg = vector_get(s->segments, i);
        if (seg->x == head->x && seg->y == head->y)
            return 1;
    }
    return 0;
}

void snake_set_position(Snake* s, int x, int y) {
    if (!s) return;

    Segment *seg =vector_get(s->segments, 0);
    if (!seg) return;

    seg->x = x;
    seg->y = y;
}

int snake_get_x(const Snake* s) {
    if (!s) return 0;
    return snake_get_segment_x(s, 0);
}

int snake_get_y(const Snake* s) {
    if (!s) return 0;
    return snake_get_segment_y(s, 0);
}

int snake_get_length(const Snake *s) {
    if (!s) return 0;
    return (int)vector_get_size(s->segments);
}

int snake_get_score(const Snake* s) {
    if (!s) return 0;
    return s->score;
}

int snake_get_segment_x(const Snake *s, int segment) {
    Segment *seg = vector_get(s->segments, segment);
    if (!seg) return 0;
    return seg->x;
}

int snake_get_segment_y(const Snake *s, int segment) {
    Segment *seg = vector_get(s->segments, segment);
    if (!seg) return 0;
    return seg->y;
}

bool snake_is_alive(const Snake* s) {
    if (!s) return false;
    return s->alive;
}

void snake_set_alive(Snake *s, bool alive) {
    if (!s) return;
    s->alive = alive;
}