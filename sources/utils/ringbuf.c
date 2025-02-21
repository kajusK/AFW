/**
 * @file    utils/ringbuf.c
 * @brief   Ring buffer
 */

#include "ringbuf.h"

bool Ring_Push(ring_t *ring, char data)
{
    if (Ring_Full(ring)) {
        return false;
    }
    ring->buffer[ring->end] = data;

    ring->end++;
    if (ring->end >= ring->length) {
        ring->end = 0;
    }

    return true;
}

char Ring_Pop(ring_t *ring)
{
    char data;

    if (Ring_Empty(ring)) {
        return -1;
    }

    data = ring->buffer[ring->start];

    ring->start++;
    if (ring->start >= ring->length) {
        ring->start = 0;
    }

    return data;
}

bool Ring_Full(ring_t *ring)
{
    uint8_t next;
    next = ring->end + 1;
    if (next >= ring->length) {
        next = 0;
    }
    if (next == ring->start) {
        return true;
    }
    return false;
}

bool Ring_Empty(const ring_t *ring)
{
    return ring->start == ring->end;
}

void Ring_Clear(ring_t *ring)
{
    while (!Ring_Empty(ring)) {
        (void)Ring_Pop(ring);
    }
}

void Ring_Init(ring_t *ring, char *buffer, uint8_t size)
{
    ring->buffer = buffer;
    ring->length = size;
    ring->start = ring->end = 0;
}
