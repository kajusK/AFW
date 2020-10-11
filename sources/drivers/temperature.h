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
 * @file    drivers/temperature.c
 * @brief   Various temperature sensors
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_TEMPERATURE_H
#define __DRIVERS_TEMPERATURE_H

#include <types.h>

/**
 * Convert LMT87 voltage to temperature
 *
 * @param voltage_mv        LMT87 output voltage in mV
 * @return Temperature in milli degrees Celsius
 */
extern int32_t LMT87_ConvertmC(uint16_t voltage_mv);

/**
 * Convert thermocouple type J voltage to temperature
 *
 * @param voltage_uv    Thermocouple voltage in uV
 * @param cold_temp_mc  Cold end temperature in milli degrees celsius
 */
extern int32_t TC_JConvertmC(uint32_t voltage_uv, int32_t cold_temp_mc);

/**
 * Convert thermocouple type K voltage to temperature
 *
 * @param voltage_uv    Thermocouple voltage in uV
 * @param cold_temp_mc  Cold end temperature in milli degrees celsius
 */
extern int32_t TC_KConvertmC(uint32_t voltage_uv, int32_t cold_temp_mc);

#endif

/** @} */
