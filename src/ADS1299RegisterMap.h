// Shadow register map for a single ADS1299 chip.
// Maintains an in-RAM copy of all 24 registers with dirty tracking.
#pragma once

#include <cstdint>
#include "ADS1299Registers.h"

namespace ads1299_config {

class RegisterMap {
public:
    RegisterMap();

    // Reset all registers to datasheet default values and clear dirty flags.
    // numChannels sets the NU_CH bits in the ID register shadow (4, 6, or 8).
    // Defaults to 8 (ADS1299) so that proxy iteration covers all channels
    // even before load() reads the actual hardware ID register.
    void resetDefaults(uint8_t numChannels = 8);

    // Set a bit field within a register.
    // reg: register address
    // mask: bit mask for the field (e.g. 0x70 for bits [6:4])
    // shift: bit position of LSB of the field
    // value: value to set (pre-shift, will be masked)
    void setField(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t value);

    // Get a bit field from a register.
    uint8_t getField(uint8_t reg, uint8_t mask, uint8_t shift) const;

    // Direct register access.
    uint8_t read(uint8_t reg) const;
    void write(uint8_t reg, uint8_t value);

    // Dirty tracking.
    bool isDirty(uint8_t reg) const;
    void clearDirty(uint8_t reg);
    void clearAllDirty();

    // Number of channels for this chip (decoded from ID register NU_CH bits).
    // Returns 4, 6, or 8.
    uint8_t numChannels() const;

private:
    // Enforce fixed bits for registers that have reserved/must-write values.
    void enforceFixedBits(uint8_t reg);

    uint8_t _regs[reg::NUM_REGISTERS];
    bool _dirty[reg::NUM_REGISTERS];
};

} // namespace ads1299_config
