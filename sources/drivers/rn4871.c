/**
 * @file    drivers/rn4871.c
 * @brief   Microchip RN4871 BLE
 *
 * http://ww1.microchip.com/downloads/en/DeviceDoc/RN4870-71-Bluetooth-Low-Energy-Module-User-Guide-DS50002466C.pdf
 */

#include <types.h>
#include <ctype.h>
#include <string.h>
#include <hal/uart.h>
#include <hal/io.h>
#include <utils/time.h>
#include <utils/ringbuf.h>
#include <utils/string.h>
#include "rn4871.h"

/** Timeout in ms for waiting on command response */
#define COMMAND_TIMEOUT_MS 500
/** Timeout to wait for prompt after command */
#define PROMPT_TIMEOUT_MS  300

/** Timeout between bytes in % % message */
#define INTER_BYTE_TIMEOUT_MS 20

/** Timeout for device reboot */
#define REBOOT_TIMEOUT_MS 5000

/* Commands */
#define CMD_SET_SERIALIZED_NAME  "S-"
#define CMD_SET_AUTH             "SA"
#define CMD_SET_BAUD             "SB"
#define CMD_SET_CONNECTABLE      "SC"
#define CMD_SET_APPEARANCE       "SDA"
#define CMD_SET_DIS_FW           "SDF"
#define CMD_SET_DIS_HW           "SDH"
#define CMD_SET_DIS_SW           "SDR"
#define CMD_SET_DIS_MODEL        "SDM"
#define CMD_SET_DIS_MANUF        "SDN"
#define CMD_SET_DIS_SERIAL       "SDS"
#define CMD_SET_FACTORY          "SF"
#define CMD_SET_ADV_POWER        "SGA"
#define CMD_SET_CON_POWER        "SGC"
#define CMD_SET_TIMER            "SM"
#define CMD_SET_NAME             "SN"
#define CMD_SET_LOW_POWER        "SO" /* requires PIO with UART_RX_IND connected */
#define CMD_SET_PIN              "SP"
#define CMD_SET_FEATURES         "SR"
#define CMD_SET_DEFAULT_SERVICES "SS" /* reboot required upon change */
#define SERVICE_DEV_INFO         0x80
#define SERVICE_TRANSP_UART      0x40
#define SERVICE_BEACON           0x20
#define CMD_SET_CON_PARAM        "ST"
#define CMD_SET_ADV_TIMEOUT      "STA"
#define CMD_SET_ADV_BEACON       "STB"
#define CMD_SET_GPIO             "SW"
#define CMD_GET_CONN_STAT        "GK"
#define CMD_GET_PEER_NAME        "GNR"
#define CMD_GET_ADC              "@"
#define CMD_GET_GPIO_VAL         "|I"
#define CMD_SET_GPIO_VAL         "|O"
#define CMD_SET_PWM              "["
#define CMD_SET_MAC              "&"
#define CMD_GET_CONNECTED        "GK"
#define CMD_CLEAR_MAC            "&C"
#define CMD_GENERATE_MAC         "&R"
#define CMD_START_ADVERTISING    "A"
#define CMD_STOP_ADVERTISING     "Y"
#define CMD_CREATE_BOND          "B" /* Bond to createa secure communication */
#define CMD_CONNECT              "C" /* Connect to last/specific device */
#define CMD_START_CENTRAL        "F" /* Switches to central mode and starts scanning */
#define CMD_ENTER_TRANS_UART     "I" /* Enter transparent uart mode */
#define CMD_SUSPEND              "O"
#define CMD_REBOOT               "R"
#define CMD_ADD_SERVICE          "PS"
#define CMD_ADD_CHAR             "PC"
#define CMD_CLEAR_SERVICES       "PZ"
#define CMD_WRITE_LOCAL          "SHW"
#define CMD_READ_LOCAL           "SHR"

#define ENTER_CMD_MODE "$$$" /* Shows CMD> upon entering */
#define LEAVE_CMD_MODE "---" /* Gets END response */
#define PROMPT         "CMD>"

/** Wait for prompt response */
#define RN4871i_WaitPrompt(desc) RN4871i_Expect((desc), PROMPT, PROMPT_TIMEOUT_MS, false)

/** Descriptor used for receiving function */
static rn4871_desc_t *rn4871i_desc;

/**
 * Process status update received
 *
 * @param desc          Device descriptor
 * @param msg           Message from the BLE device
 */
static void RN4871i_ProcessStatus(rn4871_desc_t *desc, const char *msg)
{
    rn4871_evt_t evt;
    size_t len = strlen(msg);

    if (len == 11 && strcmp("ADV_TIMEOUT", msg) == 0) {
        evt = BLE_EVT_ADV_TIMEOUT;
    } else if (len == 10 && strcmp("DISCONNECT", msg) == 0) {
        evt = BLE_EVT_DISCONNECTED;
        desc->connected = false;
    } else if (len == 6 && strcmp("REBOOT", msg) == 0) {
        evt = BLE_EVT_REBOOTED;
        desc->connected = false;
        desc->rebooted = true;
    } else if (len > 7 && strncmp("CONNECT", msg, 7) == 0) {
        evt = BLE_EVT_CONNECTED;
        desc->connected = true;
    } else {
        return;
    }

    if (desc->cb) {
        desc->cb(evt, NULL);
    }
}

/**
 * Store received character for char write command into event data structure
 *
 * The command format is WV,ABCD,01234...
 * The WV, is filtered by parent function, we start at position 3 (starts at 0)
 *
 * @param [out] data    Data structure to modify
 * @param       pos     Position in the message
 * @param       byte    Byte received
 */
static void RN4871i_ProcessWriteByte(rn4871_evt_data_t *data, uint16_t pos, uint8_t byte)
{
    ASSERT_NOT(data == NULL);

    if (pos <= 3) {
        data->len = 0;
        data->handle = 0;
    }

    if (pos <= 6) {
        data->handle |= ((uint16_t)hex2dec(byte)) << (4 * (6 - pos));
    } else if (pos >= 8 && data->len < sizeof(data->data)) {
        if (pos & 0x1) {
            data->data[data->len] |= hex2dec(byte);
            data->len++;
        } else {
            data->data[data->len] = hex2dec(byte) << 4;
        }
    }
}

/**
 * Callback for data received over uart
 *
 * Filters string between % % to RN4871i_ProcessStatus function, other
 * data are put into a ring buffer for further processing
 *
 * @param byte  Byte received
 */
static void RN4871i_UartCb(uint8_t byte)
{
    static bool in_event = false;
    static bool in_write = false;
    static char buf[32];
    static uint16_t pos;
    static uint32_t last_ts;
    static rn4871_evt_data_t data;

    if (rn4871i_desc == NULL) {
        return;
    }

    if (millis() - last_ts > INTER_BYTE_TIMEOUT_MS) {
        in_event = false;
    }
    last_ts = millis();

    if (byte == '%') {
        if (!in_event) {
            in_event = true;
            in_write = false;
            pos = 0;
        } else {
            in_event = false;
            if (!in_write) {
                buf[pos] = '\0';
                RN4871i_ProcessStatus(rn4871i_desc, buf);
            } else if (rn4871i_desc->cb != NULL) {
                rn4871i_desc->cb(BLE_EVT_WRITE, &data);
            }
        }
        return;
    }

    if (in_event) {
        if (!in_write && pos == 3 && strncmp("WV,", buf, 3) == 0) {
            in_write = true;
        }

        if (in_write) {
            RN4871i_ProcessWriteByte(&data, pos, byte);
        } else if (pos < (sizeof(buf) - 1)) {
            buf[pos] = byte;
        }
        pos++;
        return;
    }

    Ring_Push(&rn4871i_desc->rbuf, byte);
}

/**
 * Expect string response terminated by a new line
 *
 * @param desc          Device descriptor
 * @param expect        String that shall be received
 * @param timeout_ms    Timeout for waiting for response
 * @param terminated    If true, waiting for expect stops at new line char
 * @return True if string received and terminated by new line, false if
 *           something other arrived
 */
static bool RN4871i_Expect(rn4871_desc_t *desc, const char *expect, uint16_t timeout_ms,
    bool terminated)
{
    char c;
    uint8_t pos = 0;
    uint32_t start = millis();

    while (millis() - start < timeout_ms) {
        if (Ring_Empty(&desc->rbuf)) {
            continue;
        }
        c = Ring_Pop(&desc->rbuf);

        if (terminated && (c == '\n' || c == '\r')) {
            return expect[pos] == '\0';
        }
        if (c == expect[pos]) {
            pos++;
            if (!terminated && expect[pos] == '\0') {
                return true;
            }
        } else {
            pos = 0;
        }
    }

    return false;
}

/**
 * Send a generic command to RN4871
 *
 * @param desc      Device descriptor
 * @param cmd       Command to be executed
 * @param param     Optional parameters or NULL
 * @param expect    String that should be received as a response
 * @param timeout_ms    Time to wait for response
 * @return True if succeeded (expected string returned)
 */
static bool RN4871i_CmdRaw(rn4871_desc_t *desc, const char *cmd, const char *param,
    const char *expect, uint16_t timeout_ms)
{
    ASSERT_NOT(desc == NULL || cmd == NULL);

    Ring_Clear(&desc->rbuf);
    UARTd_Puts(desc->uart_device, cmd);
    if (param != NULL && strlen(param)) {
        UARTd_Putc(desc->uart_device, ',');
        UARTd_Puts(desc->uart_device, param);
    }
    UARTd_Putc(desc->uart_device, '\r');

    return RN4871i_Expect(desc, expect, timeout_ms, true);
}

/**
 * Send a classic (AOK as response) command to RN4871, wait for CMD prompt
 * afterwards
 *
 * @param desc      Device descriptor
 * @param cmd       Command to be executed
 * @param param     Optional parameters or NULL
 * @return True if succeeded (AOK returned)
 */
static bool RN4871i_Cmd(rn4871_desc_t *desc, const char *cmd, const char *param)
{
    bool ret;

    ret = RN4871i_CmdRaw(desc, cmd, param, "AOK", COMMAND_TIMEOUT_MS);
    ret &= RN4871i_WaitPrompt(desc);
    return ret;
}

/**
 * Enter the command mode
 *
 * @param desc      Device descriptor
 * @return True if succeeded
 */
static bool RN4871i_EnterCmdMode(rn4871_desc_t *desc)
{
    Ring_Clear(&desc->rbuf);
    UARTd_Puts(desc->uart_device, ENTER_CMD_MODE);
    return RN4871i_WaitPrompt(desc);
}

/**
 * Wait until the device is rebooted
 *
 * @param desc      Device descriptor
 * @param True if rebooted, false otherwise
 */
static bool RN4871i_WaitReboot(rn4871_desc_t *desc)
{
    uint32_t start = millis();
    while ((millis() - start) < REBOOT_TIMEOUT_MS && !desc->rebooted) {
        ;
    }
    if (millis() - start >= REBOOT_TIMEOUT_MS) {
        return false;
    }
    return true;
}

/**
 * Reset module to factory defaults
 *
 * @param desc      Device descriptor
 * @return True if succeeded
 */
static bool RN4871i_ResetFactory(rn4871_desc_t *desc)
{
    desc->rebooted = false;
    RN4871i_CmdRaw(desc, CMD_SET_FACTORY, "2", "Reboot after Factory Reset", COMMAND_TIMEOUT_MS);
    /* factory reset clears the baudrate */
    UARTd_SetBaudrate(desc->uart_device, RN4871_DEF_BAUDRATE);
    if (!RN4871i_WaitReboot(desc)) {
        return false;
    }
    return RN4871i_EnterCmdMode(desc);
}

/**
 * Set default services advertised by the device
 *
 * @param desc      Device descriptor
 * @param mask      Services mask, e.g. SERVICE_DEV_INFO | SERVICE_BEACON
 * @return True if succeeded
 */
static bool RN4871i_SetDefaultServices(rn4871_desc_t *desc, uint8_t mask)
{
    char buf[3];
    num2hex(mask, 2, buf);
    return RN4871i_Cmd(desc, CMD_SET_DEFAULT_SERVICES, buf);
}

/**
 * Set BLE mandatory GAP service values
 *
 * For possible appearance values, refer to
 * https://specificationrefs.bluetooth.com/assigned-values/Appearance%20Values.pdf
 *
 * @param desc          Device descriptor
 * @param name          Device name
 * @param appearance    Appearance value
 */
static bool RN4871i_SetGAPService(rn4871_desc_t *desc, const char *name, uint16_t appearance)
{
    char buf[5];
    num2hex(appearance, 4, buf);
    RN4871i_Cmd(desc, CMD_SET_APPEARANCE, buf);
    return RN4871i_Cmd(desc, CMD_SET_NAME, name);
}

/**
 * Set Device information service content
 *
 * @param desc     Device descriptor
 * @param dis      Data to fill in the service
 * @return True if succeeded
 */
static bool RN4871i_SetDeviceInformation(rn4871_desc_t *desc, const rn4871_dis_t *dis)
{
    ASSERT_NOT(desc == NULL || dis == NULL);

    RN4871i_Cmd(desc, CMD_SET_DIS_FW, dis->fw_revision);
    RN4871i_Cmd(desc, CMD_SET_DIS_HW, dis->hw_revision);
    RN4871i_Cmd(desc, CMD_SET_DIS_SW, dis->sw_revision);
    RN4871i_Cmd(desc, CMD_SET_DIS_MANUF, dis->manufacturer);
    RN4871i_Cmd(desc, CMD_SET_DIS_MODEL, dis->model_name);
    return RN4871i_Cmd(desc, CMD_SET_DIS_SERIAL, dis->serial);
}

/**
 * Convert 4 characters in buf to uint16 BLE handle
 *
 * @param buf       4 characters to be converted (doesn't care about '\0')
 * @return handle value
 */
static uint16_t RN4871i_Str2Handle(const char *buf)
{
    uint16_t handle = 0;

    for (uint8_t i = 0; i < 4; i++) {
        handle *= 16;
        handle += hex2dec(buf[i]);
    }
    return handle;
}

/**
 * Get handle for given UUID string
 *
 * @param desc      Device descriptor
 * @param uuid      UUID string (without dashes)
 * @return 0 if failed or characteristic handle
 */
static uint16_t RN4871i_GetHandle(rn4871_desc_t *desc, const char *service_uuid, const char *uuid)
{
    const char *end = "END";
    uint8_t pos_end = 0;
    uint8_t pos_uuid = 0;
    uint8_t pos_buf = 0;
    char c;
    uint32_t start = millis();
    char buf[5];

    Ring_Clear(&desc->rbuf);
    UARTd_Puts(desc->uart_device, "LS,");
    UARTd_Puts(desc->uart_device, service_uuid);
    UARTd_Puts(desc->uart_device, "\n");
    while (millis() - start < COMMAND_TIMEOUT_MS) {
        if (Ring_Empty(&desc->rbuf)) {
            continue;
        }
        c = Ring_Pop(&desc->rbuf);
        if (c == end[pos_end]) {
            pos_end++;
        } else {
            pos_end = 0;
        }
        if (end[pos_end] == '\0') {
            break;
        }

        if (uuid[pos_uuid] != '\0') {
            if (c == toupper(uuid[pos_uuid])) {
                pos_uuid++;
            } else {
                pos_uuid = 0;
            }
        } else {
            buf[pos_buf++] = c;
            if (pos_buf == 5) {
                break;
            }
        }
    }

    if (!RN4871i_WaitPrompt(desc) || pos_buf != 5) {
        return 0;
    }

    uint16_t handle = RN4871i_Str2Handle(&buf[1]);
    return handle;
}

uint16_t RN4871_AddChar(rn4871_desc_t *desc, const char *uuid, uint16_t props, uint8_t size)
{
    char buf[39];
    uint8_t len;
    ASSERT_NOT(desc == NULL || uuid == NULL);

    strncpy(buf, uuid, 32);
    buf[32] = '\0';
    len = strlen(buf);
    buf[len++] = ',';
    num2hex(props, 2, &buf[len]);
    len += 2;
    buf[len++] = ',';
    num2hex(size, 2, &buf[len]);
    len += 2;
    buf[len] = '\0';

    if (!RN4871i_Cmd(desc, CMD_ADD_CHAR, buf)) {
        return 0;
    }
    return RN4871i_GetHandle(desc, desc->last_service, uuid);
}

bool RN4871_AddService(rn4871_desc_t *desc, const char *uuid)
{
    ASSERT_NOT(desc == NULL || uuid == NULL);
    desc->last_service = uuid;
    return RN4871i_Cmd(desc, CMD_ADD_SERVICE, uuid);
}

bool RN4871_WriteChar(rn4871_desc_t *desc, uint16_t handle, const uint8_t *data, uint8_t len)
{
    bool ret;
    char buf[5];
    ASSERT_NOT(desc == NULL || data == NULL);

    Ring_Clear(&desc->rbuf);
    UARTd_Puts(desc->uart_device, CMD_WRITE_LOCAL);
    UARTd_Putc(desc->uart_device, ',');
    num2hex(handle, 4, buf);
    UARTd_Puts(desc->uart_device, buf);
    UARTd_Putc(desc->uart_device, ',');
    for (uint8_t i = 0; i < len; i++) {
        num2hex(data[i], 2, buf);
        UARTd_Puts(desc->uart_device, buf);
    }
    UARTd_Putc(desc->uart_device, '\r');

    ret = RN4871i_Expect(desc, "AOK", COMMAND_TIMEOUT_MS, true);
    ret &= RN4871i_WaitPrompt(desc);
    return ret;
}

uint8_t RN4871_ReadChar(rn4871_desc_t *desc, uint16_t handle, uint8_t *data, uint8_t len)
{
    char buf[5];
    char c;
    bool first = true;
    uint8_t bytes = 0;
    uint32_t start = millis();

    UARTd_Puts(desc->uart_device, CMD_READ_LOCAL);
    UARTd_Putc(desc->uart_device, ',');
    num2hex(handle, 4, buf);
    UARTd_Puts(desc->uart_device, buf);
    UARTd_Putc(desc->uart_device, '\r');

    while (bytes < len && millis() - start < COMMAND_TIMEOUT_MS) {
        if (Ring_Empty(&desc->rbuf)) {
            continue;
        }
        c = Ring_Pop(&desc->rbuf);
        if (c == '\n' || c == '\r') {
            break;
        }

        if (!isxdigit(c)) {
            bytes = 0;
            break;
        }

        if (first) {
            *data = hex2dec(c);
            first = false;
        } else {
            *data *= 16;
            *data += hex2dec(c);
            first = true;
            data++;
            bytes++;
        }
    }

    if (!RN4871i_WaitPrompt(desc)) {
        return 0;
    }
    return bytes;
}

bool RN4871_IsConnected(rn4871_desc_t *desc)
{
    return desc->connected;
}

bool RN4871_StartAdvertising(rn4871_desc_t *desc, uint16_t interval_ms, uint32_t timeout_ms)
{
    char buf[10];
    if (timeout_ms == 0) {
        num2hex(interval_ms, 4, buf);
    } else {
        num2hex(interval_ms, 4, buf);
        buf[4] = ',';
        num2hex((uint16_t)((uint32_t)timeout_ms * 1000 / 640), 4, &buf[5]);
    }
    return RN4871i_Cmd(desc, CMD_START_ADVERTISING, buf);
}

bool RN4871_Reboot(rn4871_desc_t *desc)
{
    desc->rebooted = false;
    RN4871i_CmdRaw(desc, CMD_REBOOT, "1", "Rebooting", COMMAND_TIMEOUT_MS);
    if (!RN4871i_WaitReboot(desc)) {
        return false;
    }
    return RN4871i_EnterCmdMode(desc);
}

bool RN4871_SetConnParam(rn4871_desc_t *desc, uint32_t min_interval_ms, uint32_t max_interval_ms,
    uint16_t latency, uint32_t timeout_ms)
{
    char buf[20];

    num2hex((uint16_t)(min_interval_ms * 100 / 125), 4, buf);
    buf[4] = ',';
    num2hex((uint16_t)(max_interval_ms * 100 / 125), 4, &buf[5]);
    buf[9] = ',';
    num2hex(latency, 4, &buf[10]);
    buf[14] = ',';
    num2hex((uint16_t)(timeout_ms / 10), 4, &buf[15]);

    return RN4871i_Cmd(desc, CMD_SET_CON_PARAM, buf);
}

bool RN4871_SetAdvIntervals(rn4871_desc_t *desc, uint16_t fast_ms, uint32_t timeout_s,
    uint16_t slow_ms, uint16_t beacon_ms)
{
    char buf[15];

    num2hex((uint16_t)((uint32_t)fast_ms * 1000 / 625), 4, buf);
    buf[4] = ',';
    num2hex((uint16_t)(timeout_s * 100 / 1024), 4, &buf[5]);
    buf[9] = ',';
    num2hex((uint16_t)((uint32_t)slow_ms * 1000 / 625), 4, &buf[10]);
    if (RN4871i_Cmd(desc, CMD_SET_ADV_TIMEOUT, buf) == false) {
        return false;
    }
    num2hex(beacon_ms * 1000 / 625, 4, buf);
    return RN4871i_Cmd(desc, CMD_SET_ADV_BEACON, buf);
}

bool RN4871_SetPower(rn4871_desc_t *desc, uint8_t adv, uint8_t con)
{
    char buf[2];

    ASSERT_NOT(desc == NULL || adv > 5 || con > 5);
    buf[0] = adv + '0';
    buf[1] = '\0';
    RN4871i_Cmd(desc, CMD_SET_ADV_POWER, buf);
    buf[0] = con + '0';
    return RN4871i_Cmd(desc, CMD_SET_CON_POWER, buf);
}

void RN4871_SetLowPower(rn4871_desc_t *desc, bool state)
{
    ASSERT_NOT(desc == NULL);

    if (!desc->low_power) {
        return;
    }
    IOd_SetLine(desc->rx_ind_port, desc->rx_ind_pad, !state);
    if (!state) {
        /* 5 ms delay is requred after waking up from the low power mode */
        delay_ms(5);
    }
}

bool RN4871_EnableLowPowerSupport(rn4871_desc_t *desc, uint32_t rx_ind_port, uint8_t rx_ind_pad)
{
    desc->low_power = true;
    desc->rx_ind_port = rx_ind_port;
    desc->rx_ind_pad = rx_ind_pad;

    IOd_SetLine(rx_ind_port, rx_ind_pad, 0);
    return RN4871i_Cmd(desc, CMD_SET_LOW_POWER, "1");
}

void RN4871_RegisterEventCb(rn4871_desc_t *desc, rn4871_evt_cb_t cb)
{
    desc->cb = cb;
}

bool RN4871_Init(rn4871_desc_t *desc, uint8_t uart_device, rn4871_baudrate_t baudrate,
    const char *name, uint16_t appearance, const rn4871_dis_t *dis)
{
    char buf[3];
    static const uint32_t baudrates[] = { 921600, 460800, 230400, 115200, 57600, 38400, 28800,
        19200, 14400, 9600, 4800, 2400 };

    ASSERT_NOT(
        desc == NULL || name == NULL || baudrate >= sizeof(baudrates) / sizeof(baudrates[0]));

    Ring_Init(&desc->rbuf, desc->rbuf_data, sizeof(desc->rbuf_data));
    rn4871i_desc = desc;
    desc->low_power = false;
    desc->uart_device = uart_device;
    desc->cb = NULL;
    desc->connected = false;
    UARTd_SetRxCallback(uart_device, RN4871i_UartCb);

    /* Try to enter cmd mode with default baudrate, if not working, try new */
    if (RN4871i_EnterCmdMode(desc) == false) {
        UARTd_SetBaudrate(desc->uart_device, baudrates[baudrate]);
        if (RN4871i_EnterCmdMode(desc) == false) {
            return false;
        }
    }

    /* Factory reset aso restarts the baudrate */
    RN4871i_ResetFactory(desc);
    RN4871i_Cmd(desc, CMD_STOP_ADVERTISING, NULL);

    /* Configure default services and stuff */
    RN4871i_SetGAPService(desc, name, appearance);
    if (dis != NULL) {
        RN4871i_SetDefaultServices(desc, SERVICE_DEV_INFO);
        RN4871i_SetDeviceInformation(desc, dis);
    } else {
        RN4871i_SetDefaultServices(desc, 0);
    }

    /* Switch to required baudrate */
    if (baudrate != BLE_BAUD_115200) {
        num2hex(baudrate, 2, buf);
        if (!RN4871i_Cmd(desc, CMD_SET_BAUD, buf)) {
            return false;
        }
    }

    /* New baudrate is applicable after reboot */
    desc->rebooted = false;
    RN4871i_CmdRaw(desc, CMD_REBOOT, "1", "Rebooting", COMMAND_TIMEOUT_MS);
    UARTd_SetBaudrate(desc->uart_device, baudrates[baudrate]);
    if (!RN4871i_WaitReboot(desc)) {
        return false;
    }
    return RN4871i_EnterCmdMode(desc);
}
