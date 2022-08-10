#ifndef twiboot_h
#define twiboot_h

#include <inttypes.h>
#include "Particle.h"
#include "crc.h"

/**
 * Helper macro to get the number of pages in the length of something.
 *
 * @param len The length of the thing.
 */
#define NUM_PAGES_IN(len) (((len % page_size) > 0) ? ((len / page_size) + 1) : (len / page_size))

/**
 * Helper macro to automatically start the wire library.
 */
#define START_WIRE         \
    if (!Wire.isEnabled()) \
    {                      \
        Wire.begin();      \
    }

/**
 * The Twiboot class is a library for communicating with the Twiboot bootloader.
 */
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
     * Initializes the Twiboot device. Stops the application from
     * automatically running and gets the device information (page size).
     *
     * @return True if the device was successfully initialized. Otherwise, false.
     */
    bool Init();

    /**
     * DEPRECEATED: Use Init() instead.
     *
     * Stops the bootloader from automatically timing out and starting the application.
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    inline bool AbortBootTimeout() { return Init(); };

    /**
     * Gets the version of the bootloader.
     *
     * @param buf The buffer to store the version in.
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool GetBootloaderVersion(char *buf);

    /**
     * Gets the chip's information
     *
     * @param signature The signature of the chip.
     * @param pageSize The size of a page in the chip, in bytes.
     * @param flashSize The size of the flash in the chip, in bytes.
     * @param eepromSize The size of the EEPROM in the chip, in bytes.
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool GetChipInfo(uint64_t *signature, uint8_t *pageSize, uint16_t *flashSize, uint16_t *eepromSize);

    // /**
    //  * Reads a single EEPROM byte from the chip
    //  *
    //  * @param addr The address to start reading from.
    //  *
    //  * @returns The byte read from the chip.
    //  * @returns 0x00 if the operation failed.
    //  */
    // uint8_t ReadEEPROMByte(uint16_t addr);

    // /**
    //  * Writes len bytes at once
    //  *
    //  * @param b The byte to write
    //  * @param addr The address to start writing to.
    //  *
    //  * @returns True if the operation was successful. Otherwise, false.
    //  */
    // inline bool WriteEEPROMByte(uint8_t b, uint16_t addr = 0)
    // {
    //     return WriteEEPROM(&b, 1, addr);
    // };

    /**
     * Writes len bytes at once
     *
     * @param addr The address to start writeing from.
     * @param buf The buffer to read from.
     * @param len The number of bytes to write.
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool WriteEEPROM(uint8_t *buf, uint8_t len, uint16_t addr = 0);

    /**
     * Reads a single flash page from the chip.
     *
     * @param buf The buffer to store the page in (at least 128 bytes).
     * @param page The page to read (zero-indexed).
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool ReadFlashPage(uint8_t *buf, uint16_t page);

    /**
     * Flashes a buffer of data to the device.
     * Starts at address of page and writes to the page at the length provided.
     *
     * @param buf The data to write.
     * @param len The length of the buffer
     * @param page The page to write to (zero-indexed).
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool WriteFlash(uint8_t *buf, int len, uint16_t page = 0);

    /**
     * DEPRECIATED: Use WriteFlash instead.
     *
     * Flashes a buffer of data to the device
     *
     * @param buf The data to write.
     * @param len The length of the buffer
     */
    inline void Flash(uint8_t *buf, int len) { WriteFlash(buf, len); }

    /**
     * Verifies that the device contains the same data as the buffer.
     * Uses the CRC16 standard to verify the data.
     * Starts at address 0x0000 and reads until the length provided.
     *
     * @param buf The data to verify that the device contains.
     * @param len The length of the data.
     *
     * @returns True if the data is verified. Otherwise, false.
     */
    bool Verify(uint8_t *buf, int len, uint16_t page = 0);

    /**
     * Exits the bootloader and starts the application.
     * Automatically lets go of the Wire buffer
     *
     * @returns True if the operation was successful. Otherwise, false.
     */
    bool Exit();

    /**
     * DEPRECEATED: Use Exit instead.
     * Starts the main (non-bootloader) application on the device. Automatically
     * lets go of the Wire buffer.
     */
    inline void JumpToApp() { Exit(); };

private:
    uint8_t addr;      // The address of the twiboot device
    uint8_t page_size; // The size of a page in the device
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