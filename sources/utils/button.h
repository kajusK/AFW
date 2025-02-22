/**
 * @file    utils/button.h
 * @brief   Button handling
 */

#ifndef __UTILS_BUTTON_H
#define __UTILS_BUTTON_H

#include <types.h>

/** Amount of debouncing runs before considering signal stable */
#define BTN_DEBOUNCE_STEPS 10
/** Long press minimum time */
#define BTN_LONG_PRESS_MS  500

typedef struct {
    uint32_t port;
    uint8_t pad;
    uint32_t start;
    uint8_t debounce;
    bool prev;
    bool inverted;
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
button_event_t Button(button_t *button);

/**
 * Initialize button structure
 *
 * @param port      MCU port to which the button is connected
 * @param pad       MCU pin to which the button is connected
 * @param inverted  If true, the button is pressed when pin is zero
 */
void Button_Init(button_t *button, uint32_t port, uint8_t pad, bool inverted);

#endif
