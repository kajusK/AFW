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
 * @file    drivers/rn4871.h
 * @brief   Microchip RN4871 BLE
 *
 * @addtogroup drivers
 * @{
 */

#ifndef __DRIVERS_RN4871_H
#define __DRIVERS_RN4871_H

#include <types.h>
#include <utils/ringbuf.h>

#define RN4871_DEF_BAUDRATE 115200

/** Properties of the characteristic */
#define BLE_PROP_INDICATE 0x20
#define BLE_PROP_NOTIFY 0x10
#define BLE_PROP_WRITE 0x08
#define BLE_PROP_WRITE_NO_RESP 0x04
#define BLE_PROP_READ 0x02

/** Events generated by the BLE stack */
typedef enum {
    BLE_EVT_ADV_TIMEOUT,        /**< Advertising timeouted */
    BLE_EVT_REBOOTED,           /**< BLE module was rebooted */
    BLE_EVT_DISCONNECTED,       /**< Disconnected from a device */
    BLE_EVT_CONNECTED,          /**< Connected to the device */
    BLE_EVT_WRITE,              /**< Write request arrived */
} rn4871_evt_t;

/** BLE event data */
typedef struct {
    uint16_t handle;            /** Characteristic handle */
    uint8_t data[20];           /** Data buffer */
    uint8_t len;                /** Lenght of the data received */
} rn4871_evt_data_t;

/**
 * Event callback
 *
 * The callback is called from the interrupt content
 *
 * @param evt       Event type
 * @param data      Event data
 */
typedef void (*rn4871_evt_cb_t)(rn4871_evt_t evt, rn4871_evt_data_t *data);

/** Device information service values */
typedef struct {
    const char *fw_revision;        /**< Firmware revision, up to 20 bytes */
    const char *hw_revision;        /**< Hardware revision, up to 20 bytes */
    const char *sw_revision;        /**< Software revision, up to 20 bytes */
    const char *model_name;         /**< Model name, up to 20 bytes */
    const char *manufacturer;       /**< Manufacturer, up to 20 bytes */
    const char *serial;             /**< Serial number, up to 20 bytes */
} rn4871_dis_t;

/** The BLE device descriptor */
typedef struct {
    uint32_t rx_ind_port;       /**< Port the UART_RX_IND is connected to */
    uint8_t rx_ind_pad;         /**< Pad the UART_RX_IND is connected to */
    bool low_power;             /**< True if initialized in low power mode */
    uint8_t uart_device;        /**< Uart device to use for communication */
    char rbuf_data[16];         /**< Ringbuf data storage */
    ring_t rbuf;                /**< Ring buffer for incoming data */
    rn4871_evt_cb_t cb;         /**< Event callback */
    bool connected;             /**< Is connected */
    bool rebooted;              /**< Recentry rebooted */
} rn4871_desc_t;

/**
 * Add a new BLE characteristic under recently added service
 *
 * Reboot required to apply changes
 *
 * @param desc      Device descriptor
 * @param uuid      UUID string of the characteristic (no dashes, same size as services UUID - 16 or 128 bit)
 * @param props     Characteristic properties
 * @param size      Size of the characteristic
 * @return Characteristic handle or 0 if failed
 */
extern uint16_t RN4871_AddChar(rn4871_desc_t *desc, const char *uuid,
        uint16_t props, uint8_t size);

/**
 * Add a new BLE service
 *
 * @param desc      Device descriptor
 * @param uuid      UUID string of the service (no dashes)
 * @return Successfulness of the operation
 */
extern bool RN4871_AddService(rn4871_desc_t *desc, const char *uuid);

/**
 * Write data to characteristic
 *
 * @param desc      Device descriptor
 * @param handle    Handle of the characteristic to write to
 * @param data      Data to be written
 * @param len       Length of the data to be writen
 * @return Successfulness of the operation
 */
extern bool RN4871_WriteChar(rn4871_desc_t *desc, uint16_t handle,
        const uint8_t *data, uint8_t len);

/**
 * Read characteristic value
 *
 * @param desc      Device descriptor
 * @param handle    Handle of the characteristic to read from
 * @param data      Data buffer
 * @param len       Length of the buffer
 * @return Amount of bytes received
 */
extern uint8_t RN4871_ReadChar(rn4871_desc_t *desc, uint16_t handle,
        uint8_t *data, uint8_t len);

/**
 * Get BLE connection status
 *
 * @param desc      Device descriptor
 * @return True if connected, false otherwise
 */
extern bool RN4871_IsConnected(rn4871_desc_t *desc);

/**
 * Start advertisements
 *
 * @param desc          Device descriptor
 * @param interval_ms   Advertising interval
 * @param timeout_ms    Advertising timeout, set to 0 for forever
 * @return Successfulness of the operation
 */
extern bool RN4871_StartAdvertising(rn4871_desc_t *desc,
        uint16_t interval_ms, uint32_t timeout_ms);

/**
 * Reboot the module to apply recent changes to the configuration
 *
 * @param desc      Device descriptor
 * @param full      If true, erase everything including private chars and script
 * @return Successfulness of the operation
 */
extern bool RN4871_Reboot(rn4871_desc_t *desc);

/**
 * Configure connection timing parameters
 *
 * For Apple devices, following criteria must be met
 * min_interval_ms >= 20
 * latency <= 4
 * max_interval_ms - min_interval_ms >= 25
 * (any_interval_ms + 20)*(Latency + 1) < timeout_ms * 8/30
 *
 * @param desc              Device descriptor
 * @param min_interval_ms   Minimum time interval between successive communications
 * @param max_interval_ms   Maximum time interval between successive communications, larger or equal to min interval
 * @param latency           Number of connection events the device is not required to communicate, less than timeout_ms/max_interval_ms - 1
 * @param timeout_ms        Maximum time between raw communications before considering link lost
 * @return Successfulness of the operation
 */
extern bool RN4871_SetConnParam(rn4871_desc_t *desc,
        uint32_t min_interval_ms, uint32_t max_interval_ms, uint16_t latency,
        uint32_t timeout_ms);

/**
 * Set advertising intervals
 *
 * @param desc          Device descriptor
 * @param fast_ms       Fast advertising interval
 * @param timeout_s     Advertising timeout
 * @param slow_ms       Slow advertising interval
 * @param beacon_ms     Beacon advertising interval
 * @return Successfulness of the operation
 */
extern bool RN4871_SetAdvIntervals(rn4871_desc_t *desc, uint16_t fast_ms,
        uint32_t timeout_s, uint16_t slow_ms, uint16_t beacon_ms);

/**
 * Set device advertising and connected transmit power
 *
 * @param desc      Device descriptor
 * @param adv       Advertising power (0-5, 0 is highest)
 * @param con       Connected power (0-5, 0 is highest)
 * @return Successfulness of the operation
 */
extern bool RN4871_SetPower(rn4871_desc_t *desc, uint8_t adv,
        uint8_t con);

/**
 * Control the low power mode of the device
 *
 * This function only makes sense when RN4871_EnableLowPowerSupport was called.
 * In the low power mode, the UART communication is disabled, before sending
 * any data, set low power to false. In low-power mode the BLE communication
 * is still running.
 *
 * @param desc          Device descriptor
 * @param state         True to enter low power mode, false to exit it
 */
extern void RN4871_SetLowPower(rn4871_desc_t *desc, bool state);

/**
 * Enable Low Power Mode in the device
 *
 * The UART_RX_IND pin must be controlled by the MCU. SetLowPower function
 * is used to control low power state.
 *
 *
 * @param desc          Device descriptor
 * @param rx_ind_port   Port to which the UART_RX_IND pin is connected
 * @param rx_ind_pad    Pad to which the UART_RX_IND pin is connected
 * @return Successfulness of the operation
 */
extern bool RN4871_EnableLowPowerSupport(rn4871_desc_t *desc,
        uint32_t rx_ind_port, uint8_t rx_ind_pad);

/**
 * Register callback for BLE events
 *
 * The callback is called from the interrupt content
 *
 * @param desc          Device descriptor
 * @param cb            Callback to call upon event
 */
extern void RN4871_RegisterEventCb(rn4871_desc_t *desc, rn4871_evt_cb_t cb);

/**
 * Initialize the RN4871 device and sets the characteristics
 *
 * WARNING - currently only one instance is possible, the uart callback
 * is shared, only last initialized descriptor will be used
 *
 * @param desc          Device descriptor
 * @param uart_device   Uart device to be used for communication (must be already initialized)
 * @param name          Name of the BLE device
 * @param appearance    Appearance of this device
 * @param dis           Device Information Service content or NULL if not used
 * @return True if succeeded (device is responding)
 */
extern bool RN4871_Init(rn4871_desc_t *desc, uint8_t uart_device,
        const char *name, uint16_t appearance, const rn4871_dis_t *dis);

#endif

/** @} */