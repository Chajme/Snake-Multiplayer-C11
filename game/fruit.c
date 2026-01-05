#include "fruit.h"

#include <stdlib.h>
#include <time.h>

struct Fruit {
    int x, y;
};

Fruit *create_fruit(int width, int height) {
    srand(time(NULL));
    Fruit *f = malloc(sizeof(Fruit));
    fruit_new_coordinates(f, width, height);
    return f;
}

void destroy_fruit(Fruit *f) {
    if (!f) return;
    free(f);
}

void fruit_set_coordinates(Fruit *f, int x, int y) {
    if (!f) return;
    f->x = x;
    f->y = y;
}

void fruit_new_coordinates(Fruit *f, int width, int height) {
    if (!f) return;
    f->x = rand() % width;
    f->y = rand() % height;
}

int fruit_get_x(const Fruit *f) {
    if (!f) return 0;
    return f->x;
}

int fruit_get_y(const Fruit *f) {
    if (!f) return 0;
    return f->y;
}