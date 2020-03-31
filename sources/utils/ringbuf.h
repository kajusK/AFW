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

#ifndef __UTILS_RINGBUF_H
#define __UTILS_RINGBUF_H

#include <types.h>

typedef struct {
    char *buffer;
    uint8_t length;
    volatile uint8_t start;
    volatile uint8_t end;
} ring_t;

/**
 * Push byte to ring buffer
 *
 * @param ring  Ring buffer to work with
 * @param date  Date byte to push
 * @return  True if succeeded (there was space in the buffer)
 */
extern bool Ring_Push(ring_t *ring, char data);

/**
 * Pop byte from ring buffer
 *
 * @param ring  Ring buffer to work with
 * @return  Byte from buffer, or -1 if buffer empty
 */
extern char Ring_Pop(ring_t *ring);

/**
 * Check if buffer is full
 *
 * @param ring  Ring buffer to work with
 * @return  True if full
 */
extern bool Ring_Full(ring_t *ring);

/**
 * Check if buffer is empty
 *
 * @param ring  Ring buffer to work with
 * @return  True if empty
 */
extern bool Ring_Empty(const ring_t *ring);

/**
 * Initialize ring buffer structure
 *
 * @param ring      Pointer to ring buffer structure to initialize
 * @param buffer    Buffer to be used with ring buffer
 * @param size      Length of the buffer
 */
extern void Ring_Init(ring_t *ring, char *buffer, uint8_t size);

#endif

/** @} */
