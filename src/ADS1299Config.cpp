#include "ADS1299Config.h"

#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdio>
#include <cstring>
#endif

namespace ads1299_config {

// Register names for showRegisters() output
static const char* const REG_NAMES[] = {
    "ID      ", "CONFIG1 ", "CONFIG2 ", "CONFIG3 ",
    "LOFF    ", "CH1SET  ", "CH2SET  ", "CH3SET  ",
    "CH4SET  ", "CH5SET  ", "CH6SET  ", "CH7SET  ",
    "CH8SET  ", "BIASP   ", "BIASN   ", "LOFFSP  ",
    "LOFFSN  ", "LOFFFLIP", "LOFFSTAP", "LOFFSTAN",
    "GPIO    ", "MISC1   ", "MISC2   ", "CONFIG4 "
};

Config::Config(Transport& transport, uint8_t numFrontends)
    : _transport(transport)
    , _maps(nullptr)
    , _numFrontends(numFrontends > MAX_FRONTENDS ? MAX_FRONTENDS : (numFrontends < 1 ? 1 : numFrontends))
    , _lastError(nullptr)
{
    _maps = new RegisterMap[_numFrontends];
}

Config::~Config() {
    delete[] _maps;
}

FrontendProxy Config::frontend(uint8_t index) {
    return FrontendProxy(*this, index);
}

FrontendsProxy Config::frontends() {
    return FrontendsProxy(*this);
}

ChannelProxy Config::channel(uint16_t globalIndex) {
    uint8_t feIdx = 0;
    uint16_t remaining = globalIndex;

    while (feIdx < _numFrontends) {
        uint8_t chCount = _maps[feIdx].numChannels();
        if (remaining < chCount) break;
        remaining -= chCount;
        feIdx++;
    }

    // Clamp to last frontend, channel 0 if out of bounds
    if (feIdx >= _numFrontends) {
        feIdx = _numFrontends - 1;
        remaining = 0;
    }

    return ChannelProxy(_maps[feIdx], (uint8_t)remaining);
}

ChannelsProxy Config::channels() {
    return ChannelsProxy(*this);
}

uint8_t Config::numFrontends() const {
    return _numFrontends;
}

uint16_t Config::totalChannels() const {
    uint16_t total = 0;
    for (uint8_t i = 0; i < _numFrontends; i++) {
        total += _maps[i].numChannels();
    }
    return total;
}

bool Config::apply() {
    _lastError = nullptr;

    for (uint8_t fe = 0; fe < _numFrontends; fe++) {
        // Send SDATAC to allow register writes.
        // Note: 4 tCLK cycles (~2 us at 2.048 MHz) required after SDATAC
        // before issuing register R/W commands. Transport must ensure this delay.
        if (!_transport.sendCommand(fe, reg::CMD_SDATAC)) {
            _lastError = "SDATAC failed";
            return false;
        }

        // Write all dirty registers (skip read-only: ID, LOFF_STATP, LOFF_STATN)
        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            if (!_maps[fe].isDirty(r)) continue;

            // Skip read-only registers
            if (reg::isReadOnly(r)) {
                _maps[fe].clearDirty(r);
                continue;
            }

            if (!_transport.writeRegister(fe, r, _maps[fe].read(r))) {
                _lastError = "writeRegister failed";
                return false;
            }
            _maps[fe].clearDirty(r);
        }
    }

    return true;
}

bool Config::apply(const Config& current) {
    _lastError = nullptr;

    // Mask for CONFIG1 bits that must be preserved from current hardware state
    constexpr uint8_t CONFIG1_PRESERVE_MASK =
        reg::CONFIG1_DAISY_EN_MASK | reg::CONFIG1_CLK_EN_MASK;

    uint8_t feCount = _numFrontends < current._numFrontends
                    ? _numFrontends : current._numFrontends;

    for (uint8_t fe = 0; fe < feCount; fe++) {
        // Send SDATAC to allow register writes.
        // Note: 4 tCLK cycles (~2 us at 2.048 MHz) required after SDATAC
        // before issuing register R/W commands. Transport must ensure this delay.
        if (!_transport.sendCommand(fe, reg::CMD_SDATAC)) {
            _lastError = "SDATAC failed";
            return false;
        }

        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            // Skip read-only registers (dirty flags cleared by clearAllDirty below)
            if (reg::isReadOnly(r))
                continue;

            uint8_t desired = _maps[fe].read(r);

            // For CONFIG1: preserve DAISY_EN and CLK_EN from current hardware
            if (r == reg::CONFIG1) {
                uint8_t preserved = current._maps[fe].read(reg::CONFIG1) & CONFIG1_PRESERVE_MASK;
                desired = (desired & ~CONFIG1_PRESERVE_MASK) | preserved;
            }

            uint8_t actual = current._maps[fe].read(r);

            if (desired != actual) {
                if (!_transport.writeRegister(fe, r, desired)) {
                    _lastError = "writeRegister failed";
                    return false;
                }
            }
        }

        _maps[fe].clearAllDirty();
    }

    return true;
}

bool Config::load() {
    _lastError = nullptr;

    for (uint8_t fe = 0; fe < _numFrontends; fe++) {
        // Send SDATAC to allow register reads.
        // Note: 4 tCLK cycles (~2 us at 2.048 MHz) required after SDATAC
        // before issuing register R/W commands. Transport must ensure this delay.
        if (!_transport.sendCommand(fe, reg::CMD_SDATAC)) {
            _lastError = "SDATAC failed during load";
            return false;
        }

        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            uint8_t val = 0;
            if (!_transport.readRegister(fe, r, val)) {
                _lastError = "readRegister failed during load";
                return false;
            }
            _maps[fe].write(r, val);
            _maps[fe].clearDirty(r);
        }
    }

    return true;
}

const char* Config::lastError() const {
    return _lastError;
}

// --- Serialization ---

size_t Config::serializedSize() const {
    return SERIAL_HEADER_SIZE + (size_t)_numFrontends * reg::NUM_REGISTERS;
}

size_t Config::serialize(uint8_t* buf, size_t bufLen) const {
    size_t needed = serializedSize();
    if (bufLen < needed) return 0;

    buf[0] = SERIAL_VERSION;
    buf[1] = _numFrontends;

    size_t offset = SERIAL_HEADER_SIZE;
    for (uint8_t fe = 0; fe < _numFrontends; fe++) {
        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            buf[offset++] = _maps[fe].read(r);
        }
    }

    return offset;
}

bool Config::deserialize(const uint8_t* buf, size_t bufLen) {
    _lastError = nullptr;

    if (bufLen < SERIAL_HEADER_SIZE) {
        _lastError = "Buffer too small for header";
        return false;
    }

    if (buf[0] != SERIAL_VERSION) {
        _lastError = "Unknown serialization version";
        return false;
    }

    uint8_t srcFrontends = buf[1];

    if (srcFrontends < _numFrontends) {
        _lastError = "Config-up not supported: source has fewer frontends than target";
        return false;
    }

    size_t needed = SERIAL_HEADER_SIZE + (size_t)srcFrontends * reg::NUM_REGISTERS;
    if (bufLen < needed) {
        _lastError = "Buffer too small for register data";
        return false;
    }

    // Reset all maps to defaults first
    for (uint8_t fe = 0; fe < _numFrontends; fe++) {
        _maps[fe].resetDefaults();
    }

    // Load frontends (config-down: only load up to _numFrontends)
    size_t offset = SERIAL_HEADER_SIZE;
    uint8_t loadCount = _numFrontends; // srcFrontends >= _numFrontends
    for (uint8_t fe = 0; fe < loadCount; fe++) {
        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            _maps[fe].write(r, buf[offset++]);
        }
    }

    return true;
}

// --- Diagnostics ---

void Config::showRegisters(Print& out) const {
    for (uint8_t fe = 0; fe < _numFrontends; fe++) {
        out.print("--- ADC ");
        out.print((int)fe);
        out.print(" (");
        out.print((int)_maps[fe].numChannels());
        out.print("ch) ---");
        out.println();

        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            char line[40];
            snprintf(line, sizeof(line), "0x%02X %s 0x%02X",
                     r, REG_NAMES[r], _maps[fe].read(r));
            out.println(line);
        }
        out.println();
    }
}

RegisterMap& Config::registerMap(uint8_t frontendIndex) {
    return _maps[frontendIndex < _numFrontends ? frontendIndex : 0];
}

const RegisterMap& Config::registerMap(uint8_t frontendIndex) const {
    return _maps[frontendIndex < _numFrontends ? frontendIndex : 0];
}

Transport& Config::transport() {
    return _transport;
}

} // namespace ads1299_config
