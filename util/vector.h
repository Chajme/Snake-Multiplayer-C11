#ifndef SNAKEGAME_VECTOR_H
#define SNAKEGAME_VECTOR_H
#include <stddef.h>

typedef void (*vec_copy)(void *dest, const void *src);
typedef void (*vec_destroy)(void *elem);
typedef void (*vec_print)(const void *elem);

typedef struct Vector Vector;

Vector* vector_new(size_t elem_size, vec_copy copy, vec_destroy destroy);
void vector_free(Vector* v);

int  vector_push_back(Vector *v, const void *elem);
void *vector_get(Vector *v, size_t index);
size_t vector_get_size(const Vector *v);
void vector_clear(Vector *v);

void vector_print(Vector *v, vec_print print);


#endif