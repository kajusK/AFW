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
 * @file    hal/io.c
 * @brief   IO driver, IO initialization,...
 *
 * @addtogroup hal
 * @{
 */

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "hal/io.h"

static void IOdi_GpioInit(uint32_t gpio, uint32_t otyper, uint32_t ospeedr,
        uint32_t pupdr, uint32_t odr, uint32_t afrl, uint32_t afrh,
        uint32_t moder)
{
    GPIO_OTYPER(gpio) = otyper;
    GPIO_OSPEEDR(gpio) = ospeedr;
    GPIO_PUPDR(gpio) = pupdr;
    GPIO_ODR(gpio) = odr;
    GPIO_AFRL(gpio) = afrl;
    GPIO_AFRH(gpio) = afrh;
    GPIO_MODER(gpio) = moder;
}

void IOd_SetLine(uint32_t port, uint8_t pad, bool value)
{
    if (value) {
        gpio_set(port, 1 << pad);
    } else {
        gpio_clear(port, 1 << pad);
    }
}

bool IOd_GetLine(uint32_t port, uint8_t pad)
{
    return gpio_get(port, 1 << pad);
}

void IOd_Init(void)
{
#ifdef VAL_GPIOA_MODER
    rcc_periph_clock_enable(RCC_GPIOA);
    IOdi_GpioInit(GPIOA, VAL_GPIOA_OTYPER, VAL_GPIOA_OSPEEDR, VAL_GPIOA_PUPDR,
            VAL_GPIOA_ODR, VAL_GPIOA_AFRL, VAL_GPIOA_AFRH, VAL_GPIOA_MODER);
#endif
#ifdef VAL_GPIOB_MODER
    rcc_periph_clock_enable(RCC_GPIOB);
    IOdi_GpioInit(GPIOB, VAL_GPIOB_OTYPER, VAL_GPIOB_OSPEEDR, VAL_GPIOB_PUPDR,
            VAL_GPIOB_ODR, VAL_GPIOB_AFRL, VAL_GPIOB_AFRH, VAL_GPIOB_MODER);
#endif
#ifdef VAL_GPIOC_MODER
    rcc_periph_clock_enable(RCC_GPIOC);
    IOdi_GpioInit(GPIOC, VAL_GPIOC_OTYPER, VAL_GPIOC_OSPEEDR, VAL_GPIOC_PUPDR,
            VAL_GPIOC_ODR, VAL_GPIOC_AFRL, VAL_GPIOC_AFRH, VAL_GPIOC_MODER);
#endif
#ifdef VAL_GPIOD_MODER
    rcc_periph_clock_enable(RCC_GPIOD);
    IOdi_GpioInit(GPIOD, VAL_GPIOD_OTYPER, VAL_GPIOD_OSPEEDR, VAL_GPIOD_PUPDR,
            VAL_GPIOD_ODR, VAL_GPIOD_AFRL, VAL_GPIOD_AFRH, VAL_GPIOD_MODER);
#endif
#ifdef VAL_GPIOE_MODER
    rcc_periph_clock_enable(RCC_GPIOE);
    IOdi_GpioInit(GPIOE, VAL_GPIOE_OTYPER, VAL_GPIOE_OSPEEDR, VAL_GPIOE_PUPDR,
            VAL_GPIOE_ODR, VAL_GPIOE_AFRL, VAL_GPIOE_AFRH, VAL_GPIOE_MODER);
#endif
#ifdef VAL_GPIOF_MODER
    rcc_periph_clock_enable(RCC_GPIOF);
    IOdi_GpioInit(GPIOF, VAL_GPIOF_OTYPER, VAL_GPIOF_OSPEEDR, VAL_GPIOF_PUPDR,
            VAL_GPIOF_ODR, VAL_GPIOF_AFRL, VAL_GPIOF_AFRH, VAL_GPIOF_MODER);
#endif
#ifdef VAL_GPIOG_MODER
    rcc_periph_clock_enable(RCC_GPIOG);
    IOdi_GpioInit(GPIOG, VAL_GPIOG_OTYPER, VAL_GPIOG_OSPEEDR, VAL_GPIOG_PUPDR,
            VAL_GPIOG_ODR, VAL_GPIOG_AFRL, VAL_GPIOG_AFRH, VAL_GPIOG_MODER);
#endif
#ifdef VAL_GPIOH_MODER
    rcc_periph_clock_enable(RCC_GPIOH);
    IOdi_GpioInit(GPIOH, VAL_GPIOH_OTYPER, VAL_GPIOH_OSPEEDR, VAL_GPIOH_PUPDR,
            VAL_GPIOH_ODR, VAL_GPIOH_AFRL, VAL_GPIOH_AFRH, VAL_GPIOH_MODER);
#endif
#ifdef VAL_GPIOI_MODER
    rcc_periph_clock_enable(RCC_GPIOI);
    IOdi_GpioInit(GPIOI, VAL_GPIOI_OTYPER, VAL_GPIOI_OSPEEDR, VAL_GPIOI_PUPDR,
            VAL_GPIOI_ODR, VAL_GPIOI_AFRL, VAL_GPIOI_AFRH, VAL_GPIOI_MODER);
#endif
#ifdef VAL_GPIOJ_MODER
    rcc_periph_clock_enable(RCC_GPIOR);
    IOdi_GpioInit(GPIOJ, VAL_GPIOJ_OTYPER, VAL_GPIOJ_OSPEEDR, VAL_GPIOJ_PUPDR,
            VAL_GPIOJ_ODR, VAL_GPIOJ_AFRL, VAL_GPIOJ_AFRH, VAL_GPIOJ_MODER);
#endif
#ifdef VAL_GPIOK_MODER
    rcc_periph_clock_enable(RCC_GPIOK);
    IOdi_GpioInit(GPIOK, VAL_GPIOK_OTYPER, VAL_GPIOK_OSPEEDR, VAL_GPIOK_PUPDR,
            VAL_GPIOK_ODR, VAL_GPIOK_AFRL, VAL_GPIOK_AFRH, VAL_GPIOK_MODER);
#endif
}

/** @} */
