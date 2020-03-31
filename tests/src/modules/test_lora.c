/*
 * Copyright (C) 2020 Jakub Kaderka
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
 * @file    modules/test_lora.c
 * @brief   Unit tests for lora.c
 *
 * @addtogroup tests
 * @{
 */

#include <string.h>
#include <main.h>
#include "utils/aes.c"
#include "modules/lora.c"

/* *****************************************************************************
 * Helpers
***************************************************************************** */
uint8_t data_received[128];
uint8_t data_rec_len;

static void send_cb(const uint8_t *data, size_t len)
{
    memcpy(data_received, data, len);
    data_rec_len = len;
}

/* *****************************************************************************
 * Tests
***************************************************************************** */
TEST_GROUP(LORA);

TEST_SETUP(LORA)
{
    memset(data_received, 0xab, sizeof(data_received));
    data_rec_len = 0;
    lorai_NwkSkey = NULL;
    lorai_DevAddr = NULL;
    lorai_AppSkey = NULL;
    Lora_ResetFrameCounters();
}

TEST_TEAR_DOWN(LORA)
{

}

TEST(LORA, PayloadEncrypt)
{
    const uint8_t key[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const uint8_t dev[] = {1, 2, 3, 4};
    uint8_t data[] = {1, 2, 3, 4};
    uint8_t exp[] = {0xCF, 0xF3, 0x0B, 0x4E};

    lorai_AppSkey = key;
    lorai_DevAddr = dev;

    Lorai_PayloadEncrypt(data, sizeof(data), 1, 1);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, data, sizeof(data));
}

TEST(LORA, GetMIC)
{
    const uint8_t key[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const uint8_t dev[] = {1, 2, 3, 4};
    uint8_t data[] = {0x40, 0x04, 0x03, 0x02, 0x01, 0x00, 0x01, 0x00, 0x01, 0x01, 0x02, 0x03, 0x04};
    uint8_t mic[4];
    uint8_t mic_exp[] = {0x3B, 0x07, 0x31, 0x82};

    lorai_NwkSkey = key;
    lorai_DevAddr = dev;

    Lorai_GetMIC(mic, data, sizeof(data), 1, true);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(mic_exp, mic, 4);
}

TEST(LORA, Send)
{
    const uint8_t nwkskey[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    const uint8_t appskey[] = {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2};
    const uint8_t devaddr[] = {1, 2, 3, 4};
    const uint8_t payload[] = {0xab, 0xcd, 0xef, 0x12, 0x34, 0x56, 0x78, 0x9a};
    const uint8_t exp[] = {0x40, 0x04, 0x03, 0x02, 0x01, 0x00, 0x39, 0x30, 0x01, 0x85, 0x6d, 0xfe, 0xa1, 0xc5, 0x29, 0xab, 0x67, 0xa1, 0xd9, 0xea, 0xa2};

    Lora_InitAbp(send_cb, devaddr, nwkskey, appskey);
    lorai_frame_tx_cnt = 12345;
    TEST_ASSERT_TRUE(Lora_Send(payload, sizeof(payload)));
    TEST_ASSERT_EQUAL(sizeof(exp), data_rec_len);
    TEST_ASSERT_EQUAL_HEX8_ARRAY(exp, data_received, sizeof(exp));
}

TEST_GROUP_RUNNER(LORA)
{
    RUN_TEST_CASE(LORA, PayloadEncrypt);
    RUN_TEST_CASE(LORA, GetMIC);
    RUN_TEST_CASE(LORA, Send);
}

void Lora_RunTests(void)
{
    RUN_TEST_GROUP(LORA);
}

/** @} */


