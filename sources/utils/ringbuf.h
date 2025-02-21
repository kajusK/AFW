/**
 * @file    utils/ringbuf.c
 * @brief   Ring buffer
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
 * @param ring      Ring buffer to work with
 * @param date      Data byte to push
 * @return  True if succeeded (there was space in the buffer)
 */
bool Ring_Push(ring_t *ring, char data);

/**
 * Pop byte from ring buffer
 *
 * @param ring      Ring buffer to work with
 * @return  Byte from buffer, or -1 if buffer empty
 */
char Ring_Pop(ring_t *ring);

/**
 * Check if buffer is full
 *
 * @param ring  Ring buffer to work with
 * @return  True if full
 */
bool Ring_Full(ring_t *ring);

/**
 * Check if buffer is empty
 *
 * @param ring  Ring buffer to work with
 * @return  True if empty
 */
bool Ring_Empty(const ring_t *ring);

/**
 * Clear the ring buffer content
 *
 * @param ring  Ring buffer to work with
 */
void Ring_Clear(ring_t *ring);

/**
 * Initialize ring buffer structure
 *
 * @param ring      Pointer to ring buffer structure to initialize
 * @param buffer    Buffer to be used with ring buffer
 * @param size      Length of the buffer
 */
void Ring_Init(ring_t *ring, char *buffer, uint8_t size);

#endif
