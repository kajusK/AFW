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
 * @file    utils/button.h
 * @brief   Button handling
 *
 * @addtogroup utils
 * @{
 */

#ifndef __UTILS_BUTTON_H
#define __UTILS_BUTTON_H

#include <types.h>

/** Amount of debouncing runs before considering signal stable */
#define BTN_DEBOUNCE_STEPS 10
/** Long press minimum time */
#define BTN_LONG_PRESS_MS 500

typedef struct {
    uint32_t port;
    uint8_t pad;
    uint32_t start;
    uint8_t debounce;
    bool prev;
} button_t;

typedef enum {
    BTN_NONE,           /**< No change */
    BTN_PRESSED,        /**< Button just pressed */
    BTN_RELEASED_SHORT, /**< Button released shortly after pressing */
    BTN_LONG_PRESS,     /**< Button hold for longer time */
    BTN_RELEASED_LONG,  /**< Button released after holding for longer time */
} button_event_t;

/**
 * Button handling
 *
 * Should be called periodically (contains debouncing) every few ms
 *
 * @param button        Button description
 * @return Button events
 */
extern button_event_t Button(button_t *button);

#endif

/** @} */
