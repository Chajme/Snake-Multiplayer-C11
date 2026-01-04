#ifndef SNAKEGAME_SNAKE_H
#define SNAKEGAME_SNAKE_H

#define MAX_SNAKE_LENGTH 64

typedef struct Segment Segment;
typedef struct Snake Snake;

Snake* snake_create(int startX, int startY);
void snake_destroy(Snake* s);

void snake_move(Snake* s, int width, int height);
void snake_set_direction(Snake* s, int direction);
void snake_set_position(Snake* s, int x, int y);
void snake_grow(Snake* s);
int snake_check_self_collision(const Snake* s);

int snake_get_x(Snake* s);
int snake_get_y(Snake* s);

int snake_get_length(Snake *s);
int snake_get_score(Snake* s);
int snake_get_segment_x(Snake *s, int segment);
int snake_get_segment_y(Snake *s, int segment);

#endif //SNAKEGAME_SNAKE_H