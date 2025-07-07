/**
 * @file    modules/log.h
 * @brief   System logging
 */

#ifndef __MODULES_LOG_H
#define __MODULES_LOG_H

#include <types.h>

typedef enum {
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR,
} log_level_t;

/**
 * Set log level
 *
 * @param level     Lowest log level to print
 */
void Log_SetLevel(log_level_t level);

/**
 * Add single log line
 *
 * Very limited subset of functionality is supported to reduce overall flash
 * usage. Only bare %%, %d, %u, %x, %s, %c and %p supported in format string
 *
 * @param level     Level of the message to be logged
 * @param source    Source of the message (custom string)
 * @param format    Printf like format
 * @param ...       Printf like arguments
 */
void Log_AddLine(log_level_t level, const char *source, const char *format, ...);

#define Log_Debug(source, format, ...)   Log_AddLine(LOG_DEBUG, source, format, ##__VA_ARGS__)
#define Log_Info(source, format, ...)    Log_AddLine(LOG_INFO, source, format, ##__VA_ARGS__)
#define Log_Warning(source, format, ...) Log_AddLine(LOG_WARNING, source, format, ##__VA_ARGS__)
#define Log_Error(source, format, ...)   Log_AddLine(LOG_ERROR, source, format, ##__VA_ARGS__)

/**
 * Write raw string to logger output
 *
 * @param str   String to write
 */
void Log_Raw(const char *str);

/**
 * Write a single character to raw logger output
 *
 * @param c   Character to write
 */
void Log_RawChar(char c);

/**
 * Initialize logging subsystem
 *
 * @param uart_device   Uart device to send logs to
 */
void Log_Init(uint8_t uart_device);

#endif
