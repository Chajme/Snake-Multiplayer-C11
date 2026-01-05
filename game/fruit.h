
#ifndef SNAKEGAMEREFACTORED_FRUIT_H
#define SNAKEGAMEREFACTORED_FRUIT_H

typedef struct Fruit Fruit;

Fruit *create_fruit(int width, int height);
void destroy_fruit(Fruit *f);

void fruit_set_coordinates(Fruit *f, int x, int y);
int fruit_get_x(const Fruit *f);
int fruit_get_y(const Fruit *f);

void fruit_new_coordinates(Fruit *f, int width, int height);

#endif //SNAKEGAMEREFACTORED_FRUIT_H