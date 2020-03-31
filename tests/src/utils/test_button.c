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
 * @file    utils/test_button.c
 * @brief   Unit tests for button.c
 *
 * @addtogroup tests
 * @{
 */

#include <string.h>
#include <main.h>

#include "utils/button.h"
static uint32_t time_ms;
static bool gpio;
static button_t btn;
/* *****************************************************************************
 * Mocks
***************************************************************************** */
static uint32_t millis(void)
{
    return time_ms;
}

static bool IOd_GetLine(uint32_t port, uint8_t pad)
{
    (void) port;
    (void) pad;
    return !gpio;
}

/* *****************************************************************************
 * Tests
***************************************************************************** */
#include "utils/button.c"
TEST_GROUP(BUTTON);

TEST_SETUP(BUTTON)
{
    time_ms = 0;
    gpio = false;
    memset((uint8_t *)&btn, 0, sizeof(btn));
}

TEST_TEAR_DOWN(BUTTON)
{

}

TEST(BUTTON, debounce)
{
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));

    gpio = true;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    gpio = false;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    gpio = true;
    gpio = false;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));

    gpio = true;
    for (int i = 0; i < BTN_DEBOUNCE_STEPS - 1; i++) {
        TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    }
    TEST_ASSERT_EQUAL(true, Buttoni_Debounce(&btn));
    TEST_ASSERT_EQUAL(true, Buttoni_Debounce(&btn));

    gpio = false;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    gpio = true;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    gpio = true;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    gpio = false;
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
    TEST_ASSERT_EQUAL(false, Buttoni_Debounce(&btn));
}

TEST(BUTTON, shortPress)
{
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
    gpio = 1;
    btn.debounce = 0;
    TEST_ASSERT_EQUAL(BTN_PRESSED, Button(&btn));

    time_ms += 10;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));

    gpio = 0;
    TEST_ASSERT_EQUAL(BTN_RELEASED_SHORT, Button(&btn));
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
}

TEST(BUTTON, LongPress)
{
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
    gpio = 1;
    btn.debounce = 0;
    TEST_ASSERT_EQUAL(BTN_PRESSED, Button(&btn));

    time_ms += 10;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));

    time_ms += BTN_LONG_PRESS_MS;
    TEST_ASSERT_EQUAL(BTN_LONG_PRESS, Button(&btn));

    time_ms += 10;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));

    gpio = 0;
    TEST_ASSERT_EQUAL(BTN_RELEASED_LONG, Button(&btn));
    time_ms += 10;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
}

TEST_GROUP_RUNNER(BUTTON)
{
    RUN_TEST_CASE(BUTTON, debounce);
    RUN_TEST_CASE(BUTTON, shortPress);
    RUN_TEST_CASE(BUTTON, LongPress);
}

void Button_RunTests(void)
{
    RUN_TEST_GROUP(BUTTON);
}

/** @} */
