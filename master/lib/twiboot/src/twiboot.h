#ifndef twiboot_h
#define twiboot_h

#include <inttypes.h>
#include "Particle.h"

#include "crc.h"

class Twiboot
{
public:
    /**
     * Construct a new Twiboot object
     */
    Twiboot();

    /**
     * Construct a new Twiboot object
     *
     * @param address The address of the Twiboot device.
     */
    Twiboot(uint8_t address);

    /**
     * Stops the bootloader from automatically timing out and starting the application.
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool AbortBootTimeout();

    /**
     * Gets the version of the bootloader.
     *
     * @param buf The buffer to store the version in.
     *
     * @returns The version of the bootloader as an ascii string.
     */
    void GetBootloaderVersion(char *buf);

    /**
     * Gets the chip's information
     *
     * @param signature The signature of the chip.
     * @param pageSize The size of a page in the chip, in bytes.
     * @param flashSize The size of the flash in the chip, in bytes.
     * @param eepromSize The size of the EEPROM in the chip, in bytes.
     */
    void GetChipInfo(uint64_t *signature, uint8_t *pageSize, uint16_t *flashSize, uint16_t *eepromSize);

    /**
     * Flashes a buffer of data to the device
     *
     * @param buf The data to write.
     * @param len The length of the buffer
     */
    void Flash(uint8_t *buf, int len);

    /**
     * Verifies that the device contains the same data as the buffer.
     * Uses the CRC16 standard to verify the data.
     *
     * @param buf The data to verify that the device contains.
     * @param len The length of the data.
     */
    bool Verify(uint8_t *buf, int len);

    /**
     * Starts the main (non-bootloader) application on the device. Automatically
     * lets go of the Wire buffer.
     */
    void JumpToApp();

private:
    uint8_t addr;
};

/* Need to include this to increase TWI/I2C buffer size */
#define TWI_BUFFER_SIZE 140

hal_i2c_config_t acquireWireBuffer()
{
    hal_i2c_config_t config = {
        .size = sizeof(hal_i2c_config_t),
        .version = HAL_I2C_CONFIG_VERSION_1,
        .rx_buffer = new (std::nothrow) uint8_t[TWI_BUFFER_SIZE],
        .rx_buffer_size = TWI_BUFFER_SIZE,
        .tx_buffer = new (std::nothrow) uint8_t[TWI_BUFFER_SIZE],
        .tx_buffer_size = TWI_BUFFER_SIZE};
    return config;
}

#endif // twiboot_h