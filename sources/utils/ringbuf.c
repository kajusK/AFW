/*
 * Copyright (C) 2019 Jakub Kaderka
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/**
 * @file    utils/ringbuf.c
 * @brief   Ring buffer
 *
 * @addtogroup utils
 * @{
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

void Ring_Init(ring_t *ring, char *buffer, uint8_t size)
{
    ring->buffer = buffer;
    ring->length = size;
    ring->start = ring->end = 0;
}

/** @} */
