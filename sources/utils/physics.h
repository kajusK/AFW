/**
 * @file    utils/physics.c
 * @brief   Physics calculations
 */

#ifndef __UTILS_PHYSICS_H
#define __UTILS_PHYSICS_H

#include <types.h>

/** International standard atmosphere pressure at sea level */
#define ISA_SEA_PRESSURE_PA 101325

/**
 * Calculate altitude from athospheric pressure
 *
 * @param presure_pa    Current pressure
 * @param
 */
extern int32_t pressureToAltM(uint32_t pressure_pa, uint32_t sea_level_pa);

#endif
