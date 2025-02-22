/**
 * @file    hal/i2c.c
 * @brief   I2C driver
 */

#include <types.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/i2c.h>
#include "utils/time.h"
#include "hal/i2c.h"

/** Timeout for sending one byte */
#define I2C_TIMEOUT_MS 2

static const uint32_t i2cdi_regs[] = {
    I2C1,
#ifdef I2C2_BASE
    I2C2,
#ifdef I2C3_BASE
    I2C3,
#endif
#endif
};

static const uint32_t i2cdi_rcc[] = {
    RCC_I2C1,
#ifdef I2C2_BASE
    RCC_I2C2,
#ifdef I2C3_BASE
    RCC_I2C3,
#endif
#endif
};

/**
 * Get I2C device address from device id
 *
 * @param device	Device ID (starts from 1)
 * @return Address of the device's base register
 */
static uint32_t I2Cdi_GetDevice(uint8_t device)
{
    ASSERT_NOT(device == 0 || device > sizeof(i2cdi_regs) / sizeof(i2cdi_regs[0]));
    return i2cdi_regs[device - 1];
}

/**
 * Get I2C device rcc register
 *
 * @param device	Device ID (starts from 1)
 * @return Address of the device's rcc register
 */
static enum rcc_periph_clken I2Cdi_GetRcc(uint8_t device)
{
    ASSERT_NOT(device == 0 || device > sizeof(i2cdi_rcc) / sizeof(i2cdi_rcc[0]));
    return i2cdi_rcc[device - 1];
}

/**
 * Disable and enable peripheral, e.g. to clear hanging busy flag
 *
 * @param i2c   I2C device base address
 */
static void I2Cdi_Restart(uint32_t i2c)
{
    /* write, read and write again to assure there's one cycle between writes */
    i2c_peripheral_disable(i2c);
    if (!(I2C_CR1(i2c) & I2C_CR1_PE)) {
        i2c_peripheral_enable(i2c);
    }
}

/**
 * Wait until ISR flag is set or error/timeout appears
 *
 * @param i2c   I2C device base address
 * @param flag  ISR register flag to check
 *
 * @return true if succeded, false if timeouted/error appeared
 */
static bool I2Cdi_WaitFlag(uint32_t i2c, uint32_t flag)
{
    uint32_t start = millis();

    while (!(I2C_ISR(i2c) & flag)) {
        if (i2c_nack(i2c)) {
            while (i2c_busy(i2c)) {
                /* bug? Sometime hangs on busy here, just reset peripheral */
                if (millis() - start > I2C_TIMEOUT_MS) {
                    I2Cdi_Restart(i2c);
                    break;
                }
            }
            return false;
        }
        /* When SCL line is e.g. shorted, timeout to avoid infinite loop */
        if (millis() - start > I2C_TIMEOUT_MS) {
            I2Cdi_Restart(i2c);
            return false;
        }
    }
    return true;
}

bool I2Cd_Transceive(uint8_t device, uint8_t address, const uint8_t *txbuf, uint8_t txlen,
    uint8_t *rxbuf, uint8_t rxlen)
{
    uint32_t i2c = I2Cdi_GetDevice(device);

    /* Clear interrupt flags */
    I2C_ICR(i2c) |= I2C_ICR_STOPCF | I2C_ICR_NACKCF;

    if (txlen) {
        i2c_set_7bit_address(i2c, address);
        i2c_set_write_transfer_dir(i2c);
        i2c_set_bytes_to_transfer(i2c, txlen);
        if (rxlen) {
            i2c_disable_autoend(i2c);
        } else {
            i2c_enable_autoend(i2c);
        }
        i2c_send_start(i2c);

        while (txlen--) {
            if (!I2Cdi_WaitFlag(i2c, I2C_ISR_TXIS)) {
                return false;
            }
            i2c_send_data(i2c, *txbuf++);
        }
        if (rxlen && !I2Cdi_WaitFlag(i2c, I2C_ISR_TC)) {
            /* Wait until last byte was send before sending start again */
            return false;
        }
    }

    if (rxlen) {
        /* Setting transfer properties */
        i2c_set_7bit_address(i2c, address);
        i2c_set_read_transfer_dir(i2c);
        i2c_set_bytes_to_transfer(i2c, rxlen);
        /* start transfer */
        i2c_send_start(i2c);
        /* important to do it afterwards to do a proper repeated start! */
        i2c_enable_autoend(i2c);

        for (size_t i = 0; i < rxlen; i++) {
            if (!I2Cdi_WaitFlag(i2c, I2C_ISR_RXNE)) {
                return false;
            }
            rxbuf[i] = i2c_get_data(i2c);
        }
    }
    return true;
}

void I2Cd_Init(uint8_t device, bool fast)
{
    enum rcc_periph_clken rcc = I2Cdi_GetRcc(device);
    uint32_t i2c = I2Cdi_GetDevice(device);
    uint8_t clk_mhz;

    rcc_periph_clock_enable(rcc);
    /*
     * bug at least on stm32f070, i2c clk period must be shorter than 100 ns
     * for fast mode - therefore use sysclk if at least 10 MHz, or fail
     */
    ASSERT_NOT(fast && rcc_ahb_frequency < 10000000UL);
    if (fast) {
        rcc_set_i2c_clock_sysclk(i2c);
        clk_mhz = rcc_ahb_frequency / 1000000;
    } else {
        rcc_set_i2c_clock_hsi(i2c);
        clk_mhz = 8;
    }

    i2c_reset(i2c);
    i2c_peripheral_disable(i2c);
    i2c_enable_analog_filter(i2c);
    i2c_set_digital_filter(i2c, 0);
    if (fast) {
        i2c_set_speed(i2c, i2c_speed_fm_400k, clk_mhz);
    } else {
        i2c_set_speed(i2c, i2c_speed_sm_100k, clk_mhz);
    }
    i2c_enable_stretching(i2c);
    i2c_set_7bit_addr_mode(i2c);
    i2c_peripheral_enable(i2c);
}
