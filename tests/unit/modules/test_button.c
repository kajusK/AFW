#include <string.h>
#include <unity.h>
#include "utils/button.c"

#define BUTTON_PORT 1234
#define BUTTON_PAD  12

static uint32_t time_ms;
static bool gpio;
static button_t btn;

uint32_t millis(void)
{
    return time_ms;
}

bool IOd_GetLine(uint32_t port, uint8_t pad)
{
    TEST_ASSERT_EQUAL(BUTTON_PORT, port);
    TEST_ASSERT_EQUAL(BUTTON_PAD, pad);
    return gpio;
}

void setUp(void)
{
    time_ms = 0;
    gpio = false;
    Button_Init(&btn, BUTTON_PORT, BUTTON_PAD, false);
}

void test_button_debounce(void)
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

void test_shortPress(void)
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

void test_LongPress(void)
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

void test_inverted(void)
{
    Button_Init(&btn, BUTTON_PORT, BUTTON_PAD, true);
    gpio = 1;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
    gpio = 0;
    btn.debounce = 0;
    TEST_ASSERT_EQUAL(BTN_PRESSED, Button(&btn));

    time_ms += 10;
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));

    gpio = 1;
    TEST_ASSERT_EQUAL(BTN_RELEASED_SHORT, Button(&btn));
    TEST_ASSERT_EQUAL(BTN_NONE, Button(&btn));
}
