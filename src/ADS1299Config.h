// ADS1299 board configuration library.
// Umbrella header — include this file to use the library.
//
// This library manages a shadow register map in RAM for one or more
// ADS1299 chips (up to 128). Call apply() to push changes to hardware
// via the Transport interface. No SPI driver is included.
#pragma once

#include "ADS1299Enums.h"
#include "ADS1299Registers.h"
#include "ADS1299Transport.h"
#include "ADS1299RegisterMap.h"
#include "ADS1299Proxies.h"

// For Print class (Arduino) or forward declaration for desktop builds
#if defined(ARDUINO)
#include <Arduino.h>
#else
#include <cstdio>
#include <cstring>
// Minimal Print shim for desktop testing
class Print {
public:
    virtual ~Print() = default;
    virtual size_t write(uint8_t c) = 0;
    virtual size_t write(const uint8_t* buf, size_t size) {
        size_t n = 0;
        while (size--) n += write(*buf++);
        return n;
    }
    size_t print(const char* s) {
        size_t n = 0;
        while (*s) n += write((uint8_t)*s++);
        return n;
    }
    size_t println(const char* s) {
        size_t n = print(s);
        n += write((uint8_t)'\n');
        return n;
    }
    size_t print(int val) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%d", val);
        return print(buf);
    }
    size_t println(int val) {
        size_t n = print(val);
        n += write((uint8_t)'\n');
        return n;
    }
    size_t println() { return write((uint8_t)'\n'); }
};
#endif

namespace ads1299_config {

constexpr uint8_t MAX_FRONTENDS = 128;

// Serialization wire format version
constexpr uint8_t SERIAL_VERSION = 0x01;

// Serialization header size: version(1) + numFrontends(1)
constexpr size_t SERIAL_HEADER_SIZE = 2;

class Config {
public:
    // Construct with a transport implementation and the number of ADC chips.
    // numFrontends: number of ADS1299 chips on the board (1-128)
    // Dynamically allocates register maps.
    Config(Transport& transport, uint8_t numFrontends);

    // Destructor — frees dynamically allocated register maps.
    ~Config();

    // Non-copyable (owns dynamically allocated memory + transport reference)
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;

    // --- Proxy factory methods ---

    // Access a single frontend by index (0-based).
    FrontendProxy frontend(uint8_t index);

    // Access all frontends as a broadcast proxy.
    FrontendsProxy frontends();

    // Access a single channel by global index (0-based across all frontends).
    // Global index maps: 0..N-1 for frontend 0, N..2N-1 for frontend 1, etc.
    // where N = channelsNumber() per frontend (8 for daisy-chained chips).
    ChannelProxy channel(uint16_t globalIndex);

    // Access all channels as a broadcast proxy.
    ChannelsProxy channels();

    // --- Board info ---

    // Number of ADC chips on the board.
    uint8_t numFrontends() const;

    // Total number of channels across all frontends.
    uint16_t totalChannels() const;

    // --- Apply / Load ---

    // Push all dirty shadow registers to hardware via transport.
    // Sends SDATAC before writing, writes only dirty registers,
    // then clears dirty flags.
    // Returns false and sets lastError() on failure.
    bool apply();

    // Diff-based apply: compares this config against `current` (which
    // represents the current hardware state, e.g. from load()) and
    // only writes registers that differ.
    // CONFIG1 DAISY_EN and CLK_EN bits are preserved from `current`
    // and not overwritten.
    // Returns false and sets lastError() on failure.
    bool apply(const Config& current);

    // Read all registers from hardware into the shadow map.
    // Sends SDATAC, reads all registers for all frontends, clears dirty flags.
    // Returns false and sets lastError() on transport failure.
    bool load();

    // Last error message (nullptr if no error).
    const char* lastError() const;

    // --- Serialization ---

    // Returns required buffer size for serialization.
    // Formula: 2 + numFrontends * 24
    size_t serializedSize() const;

    // Serialize config to buffer. Returns bytes written, or 0 on error.
    // Buffer must be at least serializedSize() bytes.
    size_t serialize(uint8_t* buf, size_t bufLen) const;

    // Deserialize from buffer into this Config.
    // Config-down (source frontends >= this numFrontends): succeeds,
    //   loads first numFrontends frontends from source, ignores extras.
    // Config-up (source frontends < this numFrontends): fails,
    //   returns false, sets lastError().
    // On success, marks all loaded registers as dirty.
    bool deserialize(const uint8_t* buf, size_t bufLen);

    // --- Diagnostics ---

    // Print shadow register map to a Print stream (e.g. Serial).
    void showRegisters(Print& out) const;

    // --- Internal (used by proxy classes) ---
    RegisterMap& registerMap(uint8_t frontendIndex);
    const RegisterMap& registerMap(uint8_t frontendIndex) const;
    Transport& transport();

private:
    Transport& _transport;
    RegisterMap* _maps;  // dynamically allocated array
    uint8_t _numFrontends;
    const char* _lastError;
};

} // namespace ads1299_config
