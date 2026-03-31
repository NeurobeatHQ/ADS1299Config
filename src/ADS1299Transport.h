// Abstract transport interface for ADS1299 register access.
// Users implement this to wrap their SPI driver.
#pragma once

#include <stdint.h>

namespace ads1299_config {

class Transport {
public:
    virtual ~Transport() = default;

    // Write a single register value on a given frontend (ADS1299 chip).
    // frontendIndex: 0-based index of the ADS1299 chip on the board
    // reg: register address (0x00-0x17)
    // value: byte to write
    // Returns true on success.
    virtual bool writeRegister(uint8_t frontendIndex, uint8_t reg, uint8_t value) = 0;

    // Read a single register value from a given frontend.
    // frontendIndex: 0-based index of the ADS1299 chip
    // reg: register address (0x00-0x17)
    // value: output byte
    // Returns true on success.
    virtual bool readRegister(uint8_t frontendIndex, uint8_t reg, uint8_t& value) = 0;

    // Send a command byte to a given frontend (e.g. SDATAC, RESET).
    // frontendIndex: 0-based index of the ADS1299 chip
    // cmd: command byte (see ADS1299Registers.h CMD_* constants)
    // Returns true on success.
    virtual bool sendCommand(uint8_t frontendIndex, uint8_t cmd) = 0;
};

} // namespace ads1299_config
