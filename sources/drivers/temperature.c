/**
 * @file    drivers/temperature.c
 * @brief   Various temperature sensors
 */

#include <types.h>
#include "temperature.h"

static int32_t TCi_GetVoltage(const int32_t lookup[][2], uint16_t lookup_size, int32_t cold_temp_mc)
{
    uint16_t i;
    int32_t x1, x2, y1, y2;

    /* Find a two points to interpolate between */
    for (i = 0; i < lookup_size; i++) {
        if (cold_temp_mc / 1000 < lookup[i][0]) {
            break;
        }
    }
    if (i == 0) {
        i = 1;
    }
    x1 = lookup[i - 1][0] * 1000;
    x2 = lookup[i][0] * 1000;
    y1 = lookup[i - 1][1];
    y2 = lookup[i][1];

    /* Simple linear line interpolation */
    return ((y1 - y2) * (cold_temp_mc - x1)) / (x1 - x2) + y1;
}

static int32_t TCi_GetTemp(const int32_t lookup[][2], uint16_t lookup_size, int32_t voltage_uv,
    int32_t cold_temp_mc)
{
    uint16_t i;
    int32_t x1, x2, y1, y2;

    int32_t cold_uv = TCi_GetVoltage(lookup, lookup_size, cold_temp_mc);
    /* The measured voltage = Utc - Ucold */
    voltage_uv += cold_uv;

    /* Find a two points to interpolate between */
    for (i = 0; i < lookup_size; i++) {
        if (voltage_uv < lookup[i][1]) {
            break;
        }
    }
    if (i == 0) {
        i = 1;
    }
    x1 = lookup[i - 1][1];
    x2 = lookup[i][1];
    y1 = lookup[i - 1][0] * 1000;
    y2 = lookup[i][0] * 1000;

    /* Simple linear line interpolation */
    return ((y1 - y2) * (voltage_uv - x1)) / (x1 - x2) + y1;
}

int32_t TC_JConvertmC(uint32_t voltage_uv, int32_t cold_temp_mc)
{
    const int32_t lookup[][2] = {
        { -200, -7890 },
        { -150, -6500 },
        { -100, -4633 },
        { -50,  -2431 },
        { 0,    0     },
        { 50,   2585  },
        { 100,  5269  },
        { 150,  8010  },
        { 200,  10779 },
        { 250,  13555 },
        { 300,  16327 },
        { 400,  21848 },
        { 500,  27393 },
        { 600,  33102 },
        { 700,  39132 },
        { 800,  45494 }
    };

    return TCi_GetTemp(lookup, sizeof(lookup) / sizeof(lookup[0]), voltage_uv, cold_temp_mc);
}

int32_t TC_KConvertmC(uint32_t voltage_uv, int32_t cold_temp_mc)
{
    /* Lookup table {temperature degrees C, voltage uV} */
    const int32_t lookup[][2] = {
        { -200, -5891 },
        { -100, -3554 },
        { -50,  -1889 },
        { 0,    0     },
        { 50,   2023  },
        { 100,  4096  },
        { 150,  6138  },
        { 200,  8138  },
        { 300,  12209 },
        { 400,  16397 },
        { 500,  20644 },
        { 600,  24905 },
        { 700,  29129 },
        { 800,  33275 },
        { 900,  37326 },
        { 1000, 41276 },
        { 1100, 45119 },
        { 1200, 48838 },
        { 1300, 52410 },
        { 1370, 54819 }
    };
    return TCi_GetTemp(lookup, sizeof(lookup) / sizeof(lookup[0]), voltage_uv, cold_temp_mc);
}

int32_t LMT87_ConvertmC(uint16_t voltage_mv)
{
    /** Temperature lookup table {temperature degrees C, voltage mV },
     * based on https://www.ti.com/lit/ds/symlink/lmt87.pdf */
    const int16_t lookup[][2] = {
        { -50, 3277 },
        { -40, 3160 },
        { -30, 3030 },
        { -20, 2899 },
        { -10, 2767 },
        { 0,   2633 },
        { 10,  2500 },
        { 20,  2365 },
        { 30,  2231 },
        { 40,  2095 },
        { 50,  1958 },
        { 60,  1819 },
        { 70,  1679 },
        { 80,  1539 },
        { 90,  1399 },
        { 100, 1257 },
        { 110, 1115 },
        { 120, 973  },
        { 130, 829  },
        { 140, 684  },
        { 150, 538  }
    };
    uint16_t i;
    int16_t x1, x2, y1, y2;

    /* Find a two points to interpolate between */
    for (i = 0; i < sizeof(lookup) / sizeof(lookup[0]); i++) {
        if (voltage_mv > lookup[i][1]) {
            break;
        }
    }
    /* corner case */
    if (i == 0) {
        i = 1;
    }
    x1 = lookup[i - 1][1];
    x2 = lookup[i][1];
    y1 = lookup[i - 1][0];
    y2 = lookup[i][0];

    /* Simple linear line interpolation */
    return ((y1 - y2) * (voltage_mv - x1) * 1000) / (x1 - x2) + y1 * 1000;
}
