/**
 * @file    config_items.h
 * @brief   Configuration values definition
 *
 * Generated by AFW/tools/config_items.py
 */

#ifndef __CONFIG_ITEMS_H
#define __CONFIG_ITEMS_H

#include <types.h>

/* Max length of a string stored */
#define CONFIG_STRING_MAX_LEN 16

/** int config options */
typedef enum {
    CONFIG_INT_TEST1, /* First int test */
    CONFIG_INT_TEST2, /* Second int test */
    CONFIG_INT_COUNT
} config_item_int_t;

/** Default values for the int config items */
#define CONFIG_INT_DEFAULTS {42, -1024, }

/** float config options */
typedef enum {
    CONFIG_FLOAT_TEST1, /* First float test */
    CONFIG_FLOAT_TEST2,
    CONFIG_FLOAT_COUNT
} config_item_float_t;

/** Default values for the float config items */
#define CONFIG_FLOAT_DEFAULTS {42.43, -123.456, }

/** string config options */
typedef enum {
    CONFIG_STRING_TEST1, /* test string */
    CONFIG_STRING_TEST2,
    CONFIG_STRING_COUNT
} config_item_string_t;

/** Default values for the string config items */
#define CONFIG_STRING_DEFAULTS {"Hello world", "Watch the cat!", }

/** ptr config options */
typedef enum {
    CONFIG_PTR_TEST1,
    CONFIG_PTR_TEST2,
    CONFIG_PTR_COUNT
} config_item_ptr_t;

/** Default values for the ptr config items */
#define CONFIG_PTR_DEFAULTS {"foo", "bar", }

/** Bool config options */
typedef enum {
    CONFIG_BOOL_TEST1, 
    CONFIG_BOOL_TEST2, 
    CONFIG_BOOL_TEST3, 
    CONFIG_BOOL_TEST4, 
    CONFIG_BOOL_TEST5, 
    CONFIG_BOOL_TEST6, 
    CONFIG_BOOL_TEST7, 
    CONFIG_BOOL_TEST8, 
    CONFIG_BOOL_TEST9, 
    CONFIG_BOOL_TEST10, 
    CONFIG_BOOL_TEST11, 
    CONFIG_BOOL_TEST12, 
    CONFIG_BOOL_COUNT
} config_item_bool_t;

/** Default values for the bool config items */
#define CONFIG_BOOL_DEFAULTS {0x02,0x04,}

#endif
