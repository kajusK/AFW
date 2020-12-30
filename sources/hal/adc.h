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

/** Channel with thermometer connected */
#define ADC_TEMP_CHANNEL    16
/** Channel with internal voltage reference (for measuring Vcc) */
#define ADC_INT_REF_CHANNEL 17

/** Typical voltage of the internal reference */
#define VREF_TYP_MV 1230

/**
 * Measure channel and get a raw ADC output value
 *
 * Do not use while using a DMA mode. The call is blocking.
 *
 * @param channel   Channel to read
 * @return Raw ADC 12 bit value
 */
uint16_t Adcd_ReadRaw(uint8_t channel);

/**
 * Convert raw value to mV
 *
 * @param raw       Raw data measured by ADC
 * @return Converted voltage in mV
 */
extern uint16_t Adcd_RawToMv(uint16_t raw);

/**
 * Convert raw value from internal reference to Vcc in mV
 *
 * @param raw       Raw data from the internal reference measurement
 * @return Converted MCU supply voltage in mV
 */
extern uint16_t Adcd_RawToVcc(uint16_t raw);

/**
 * Convert raw value from the internal temperature sensor to degrees C
 *
 * @param raw       Raw data from the internal temperature sensor
 * @return Temperature in degrees C
 */
extern int8_t Adcd_RawToTemp(uint16_t raw);

/**
 * Update the Vdda used in calculations to value measured from internal reference
 *
 * Should be called from time to time if vdda is not very stable
 *
 * @param raw       Raw data from the internal reference measurement
 */
extern void Adcd_UpdateVddaRaw(uint16_t raw);

/**
 * Read voltage on the channel in mV
 *
 * @param channel       Channel to read data from
 * @return voltage in mV
 */
#define Adcd_ReadMv(channel) Adcd_RawToMv(Adcd_ReadRaw(channel))

/**
 * Read power supply voltage
 *
 * @return Vdda voltage in mv
 */
#define Adcd_ReadVccMv() Adcd_RawToVcc(Adcdi_ReadRaw(ADC_INT_REF_CHANNEL))

/**
 * Read core temeperature
 *
 * @return temperature in degreec C
 */
#define Adcd_ReadTempDegC() Adcd_RawToTemp(Adcd_ReadRaw(ADC_TEMP_CHANNEL))

/**
 * Update reference voltage from internal reference measurements
 *
 * Should be called from time to time if vdda is not very stable
 */
#define Adcd_UpdateVdda() Adcd_UpdateVddaRaw(Adcd_ReadRaw(ADC_INT_REF_CHANNEL))

/**
 * Put adc peripheral to low power mode
 */
extern void Adcd_Sleep(void);

/**
 * Put adc peripheral to normal mode
 *
 * It takes a while for ADC to start, take first measurement after few dozens
 * of ms. Additionally, call Adcd_UpdateVdda to measure a real supply voltage
 * else the Adcd_ReadMv will assume the Vdda is 3,3 V
 */
extern void Adcd_Wakeup(void);

/**
 * Check if the data stored by DMA are valid (first measurement finished)
 *
 * The valid flag in the DMA is cleared, will be set after whole group is
 * measured again
 *
 * @return true if data valid
 */
extern bool Adcd_DmaDataValid(void);

/**
 * Initialize ADC in DMA mode
 *
 * When in DMA mode, no direct measuring functions should be called, use
 * raw variants instead to convert the measured value to something meaningful.
 *
 * The data are measured automatically in a loop and stored by DMA to memory
 * @param data      Memory area to store DMA results to
 * @param channels  Channels to measure - on F0 the channels must be sorted (1 to 18)
 * @param length    Amount of channels to measure
 */
extern void Adcd_InitDma(uint16_t *data, const uint8_t *channels, uint8_t length);

/**
 * Initialize the adc driver in single measurement mode
 *
 * It takes a while for ADC to start, take first measurement after few dozens
 * of ms. Additionally, call Adcd_UpdateVdda to measure a real supply voltage
 * else the Adcd_ReadMv will assume the Vdda is 3,3 V
 */
extern void Adcd_Init(void);

#endif

/** @} */
