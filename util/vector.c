#include "vector.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 4

/* Full Vector struct is private to vector.c */
struct Vector {
    void *data;
    size_t size;
    size_t capacity;
    size_t elem_size;

    vec_copy copy;
    vec_destroy destroy;
};

/* Internal functions */
static int vector_init(Vector *v, size_t elem_size,
                       vec_copy copy, vec_destroy destroy)
{
    if (elem_size == 0) return 0;

    v->size = 0;
    v->capacity = INITIAL_CAPACITY;
    v->elem_size = elem_size;
    v->copy = copy;
    v->destroy = destroy;

    v->data = malloc(v->capacity * elem_size);
    return v->data != NULL;
}

static int vector_resize(Vector *v, size_t new_capacity)
{
    void *new_data = realloc(v->data, new_capacity * v->elem_size);
    if (!new_data) return 0;

    v->data = new_data;
    v->capacity = new_capacity;
    return 1;
}

/* Public OOP-style constructor */
Vector* vector_new(size_t elem_size, vec_copy copy, vec_destroy destroy)
{
    Vector *v = malloc(sizeof(Vector));
    if (!v) return NULL;
    if (!vector_init(v, elem_size, copy, destroy)) {
        free(v);
        return NULL;
    }
    return v;
}

/* Public OOP-style destructor */
void vector_free(Vector* v)
{
    if (!v) return;

    if (v->destroy) {
        for (size_t i = 0; i < v->size; ++i) {
            void *elem = (char*)v->data + i * v->elem_size;
            v->destroy(elem);
        }
    }

    free(v->data);
    free(v);
}

/* API */
int vector_push_back(Vector *v, const void *elem)
{
    if (!v || !elem) return 0;

    if (v->size == v->capacity)
        if (!vector_resize(v, v->capacity * 2)) return 0;

    void *target = (char*)v->data + v->size * v->elem_size;

    if (v->copy) v->copy(target, elem);
    else memcpy(target, elem, v->elem_size);

    v->size++;
    return 1;
}

void *vector_get(Vector *v, size_t index)
{
    if (!v || index >= v->size) return NULL;
    return (char*)v->data + index * v->elem_size;
}

size_t vector_get_size(Vector *v)
{
    return v ? v->size : 0;
}

void vector_clear(Vector *v)
{
    if (!v) return;

    if (v->destroy) {
        for (size_t i = 0; i < v->size; ++i) {
            void *elem = (char*)v->data + i * v->elem_size;
            v->destroy(elem);
        }
    }
    v->size = 0;
}

void vector_print(Vector *v, vec_print print) {
    if (!v || !print) return;

    for (size_t i = 0; i < v->size; i++) {
        void *elem = (char*)v->data + i * v->elem_size;
        print(elem);
    }
    printf("\n");
}
