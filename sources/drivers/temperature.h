/**
 * @file    drivers/temperature.c
 * @brief   Various temperature sensors
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
