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
 * @file    utils/button.c
 * @brief   Button handling
 *
 * @addtogroup utils
 * @{
 */

#include "hal/io.h"
#include "utils/time.h"
#include "button.h"

/**
 * Debounce button signal
 *
 * Signal must be stable for BTN_DEBOUNCE_STEPS before considering pressed
 *
 * @param button    Button description
 * @return true if pressed, false otherwise
 */
static bool Buttoni_Debounce(button_t *button)
{
    bool current = !IOd_GetLine(button->port, button->pad);

    if (!current) {
        button->debounce = BTN_DEBOUNCE_STEPS - 1;
        return false;
    }

    if (button->debounce == 0) {
        return true;
    }

    button->debounce--;
    return false;
}

button_event_t Button(button_t *button)
{
    bool current = Buttoni_Debounce(button);

    /* No change */
    if (button->prev == current) {
        if (current && button->start != 0 &&
                (millis() - button->start) >= BTN_LONG_PRESS_MS) {
            button->start = 0;
            return BTN_LONG_PRESS;
        }
        return BTN_NONE;
    }

    button->prev = current;
    /* just pressed */
    if (current) {
        button->start = millis();
        /* 0 in start has special meaning */
        if (button->start == 0) {
            button->start = 1;
        }
        return BTN_PRESSED;
    }

    if (button->start == 0) {
        return BTN_RELEASED_LONG;
    }
    return BTN_RELEASED_SHORT;
}

/** @} */
