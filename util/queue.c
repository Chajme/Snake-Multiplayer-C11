#include "queue.h"

#include <stdlib.h>
#include <string.h>

struct Queue {
    void *data;
    size_t elem_size;
    size_t capacity;
    size_t front;
    size_t back;
    size_t size;
};

Queue* queue_new(size_t elem_size, size_t capacity) {
    if (elem_size == 0 || capacity == 0) return NULL;

    Queue *q = malloc(sizeof(Queue));
    if (!q) return NULL;

    q->data = malloc(elem_size * capacity);
    if (!q->data) {
        free(q);
        return NULL;
    }

    q->elem_size = elem_size;
    q->capacity = capacity;
    q->front = 0;
    q->back = 0;
    q->size = 0;

    return q;
}

void queue_free(Queue *q) {
    if (!q) return;

    free(q->data);
    free(q);
}

int queue_push(Queue *q, const void *elem) {
    if (!q || !elem || q->size == q->capacity) return 0;

    void *target = (char*)q->data + q->back * q->elem_size;
    memcpy(target, elem, q->elem_size);
    q->back = (q->back + 1) % q->capacity;
    q->size++;

    return 1;
}

int queue_pop(Queue *q, void *out_elem) {
    if (!q || !out_elem || q->size == 0) return 0;

    void *source = (char*)q->data + q->front * q->elem_size;
    memcpy(out_elem, source, q->elem_size);
    q->front = (q->front + 1) % q->capacity;
    q->size--;

    return 1;
}

int queue_is_empty(const Queue *q) {
    if (!q) return 1;
    return q->size == 0;
}
