

#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdint.h>

/*
A very simple circular buffer.

Example:
QUEUE(test, int, count)

Creates:
struct queue_test {...};
static inline void queue_test_init(struct queue_test *) {...}
static inline int queue_test_push(struct queue_test *, int *) {...}
static inline int queue_test_pop(struct queue_test *, int *) {...}

API:
queue_*_init initializes a queue
queue_*_push pushes an item onto the queue, returns 0 if successful, not 0 if fail
queue_*_pop pops an item from the queue, returns 0 if successful, not 0 if fail

*/

#define QUEUE(name, type, size) \
struct queue_##name { \
    type storage[size]; \
    /*index of the read head, initialy 0*/ \
    size_t read; \
    /*index of the write head, initialy 0*/ \
    size_t write; \
    /*number of items in the queue*/ \
    size_t count; \
}; \
static inline void queue_##name##_init(volatile struct queue_##name *q) { \
    q->read = 0; \
    q->write = 0; \
    q->count = 0; \
} \
static inline int queue_##name##_push(volatile struct queue_##name *q, const volatile type *item) { \
    if (q->count < size) { \
        size_t next = (q->write + 1) % size; \
        q->storage[next] = *item; \
        q->write = next; \
        q->count++; \
        return 0; \
    } else { \
        return -1; \
    }\
} \
static inline int queue_##name##_pop(volatile struct queue_##name *q, volatile type *item) { \
    if (q->count > 0) { \
        uint8_t next = (q->read + 1) % size; \
        *item = q->storage[next]; \
        q->read = next; \
        q->count--; \
        return 0; \
    } else { \
        return -1; \
    } \
}


#endif //QUEUE_H

