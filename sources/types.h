/**
 * @file    types.h
 * @brief   Standard types and common defines
 */

#ifndef __TYPES_H
#define __TYPES_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h> // NULL

#ifndef bool
    #define bool _Bool
    #define true 1
    #define false 0
#endif

#ifndef ASSERT
    #define ASSERT(condition) \
        if (!(condition)) { \
            while (1); \
        }
#endif

#ifndef ASSERT_NOT
    #define ASSERT_NOT(condition) ASSERT(!(condition))
#endif

#endif
