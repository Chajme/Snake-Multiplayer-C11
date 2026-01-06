#ifndef SNAKEGAME_QUEUE_H
#define SNAKEGAME_QUEUE_H
#include <stddef.h>

typedef struct Queue Queue;

Queue* queue_new(size_t elem_size, size_t capacity);
void queue_free(Queue *q);

int queue_push(Queue *q, const void *elem);
int queue_pop(Queue *q, void *out_elem);
int queue_is_empty(const Queue *q);

#endif