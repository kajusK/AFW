/**
 * @file    utils/button.c
 * @brief   Button handling
 */

#include <types.h>
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
    bool current = IOd_GetLine(button->port, button->pad);
    if (button->inverted) {
        current = !current;
    }

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
    bool current;
    ASSERT_NOT(button == NULL);

    current = Buttoni_Debounce(button);

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

void Button_Init(button_t *button, uint32_t port, uint8_t pad, bool inverted)
{
    ASSERT_NOT(button == NULL);
    button->port = port;
    button->pad = pad;
    button->inverted = inverted;
}
