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
 * @file    hal/adc.h
 * @brief   ADC driver
 *
 * @addtogroup hal
 * @{
 */

#ifndef __HAL_ADC_H
#define __HAL_ADC_H

#include <types.h>

/**
 * Read voltage on the channel in mV
 *
 * @param channel       Channel to read data from
 * @return voltage in mV
 */
extern uint16_t Adcd_ReadMv(uint8_t channel);

/**
 * Read power supply voltage
 *
 * @return Vdda voltage in mv
 */
extern uint16_t Adcd_ReadVccMv(void);

/**
 * Read core temeperature
 *
 * @return temperature in degreec C
 */
extern int8_t Adcd_ReadTempDegC(void);

/**
 * Update reference voltage from internal reference measurements
 *
 * Should be called from time to time if vdda is not very stable
 */
extern void Adcd_UpdateVdda(void);

/**
 * Put adc peripheral to low power mode
 */
extern void Adcd_Sleep(void);

/**
 * Put adc peripheral to normal mode
 */
extern void Adcd_Wakeup(void);

/**
 * Initialize the adc driver in single measurement mode
 */
extern void Adcd_Init(void);

#endif

/** @} */
