
#ifndef GAME_BACKUP_2_VECTOR_H
#define GAME_BACKUP_2_VECTOR_H
#include <stddef.h>

/* Function pointer types */
typedef void (*vec_copy_fn)(void *dest, const void *src);
typedef void (*vec_destroy_fn)(void *elem);

typedef struct Vector Vector;

/* OOP-style constructors */
Vector* vector_new(size_t elem_size, vec_copy_fn copy, vec_destroy_fn destroy);
void vector_free(Vector* v);

/* API */
int  vector_push_back(Vector *v, const void *elem);
void *vector_get(Vector *v, size_t index);
size_t vector_get_size(Vector *v);
void vector_clear(Vector *v);


#endif //GAME_BACKUP_2_VECTOR_H