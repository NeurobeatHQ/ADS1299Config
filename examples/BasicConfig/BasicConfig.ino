// BasicConfig.ino — Example of using the ADS1299Config library.
//
// This example shows how to configure a board with two ADS1299 chips
// (16 channels) for biopotential acquisition. Each chip has its own CS
// pin for register configuration. Daisy chaining is used separately for
// continuous data readback (not for register config).

#include <SPI.h>
#include <ADS1299Config.h>

using namespace ads1299_config;

// --- Implement the Transport interface for your SPI driver ---
// Each ADS1299 chip is addressed by its own CS pin.
// The frontendIndex parameter selects which CS pin to assert.
class MySpiTransport : public Transport {
public:
    MySpiTransport(const int* csPins, uint8_t numPins)
        : _csPins(csPins), _numPins(numPins)
    {
        for (uint8_t i = 0; i < _numPins; i++) {
            pinMode(_csPins[i], OUTPUT);
            digitalWrite(_csPins[i], HIGH);
        }
    }

    bool writeRegister(uint8_t frontendIndex, uint8_t reg, uint8_t value) override {
        if (frontendIndex >= _numPins) return false;
        int cs = _csPins[frontendIndex];
        // ADS1299 WREG command: 0x40 | reg, then 0x00 (1 register), then value
        digitalWrite(cs, LOW);
        SPI.transfer(0x40 | reg);
        SPI.transfer(0x00);
        SPI.transfer(value);
        digitalWrite(cs, HIGH);
        delayMicroseconds(2);  // tSDECODE
        return true;
    }

    bool readRegister(uint8_t frontendIndex, uint8_t reg, uint8_t& value) override {
        if (frontendIndex >= _numPins) return false;
        int cs = _csPins[frontendIndex];
        // ADS1299 RREG command: 0x20 | reg, then 0x00 (1 register), then read
        digitalWrite(cs, LOW);
        SPI.transfer(0x20 | reg);
        SPI.transfer(0x00);
        value = SPI.transfer(0x00);
        digitalWrite(cs, HIGH);
        delayMicroseconds(2);
        return true;
    }

    bool sendCommand(uint8_t frontendIndex, uint8_t cmd) override {
        if (frontendIndex >= _numPins) return false;
        int cs = _csPins[frontendIndex];
        digitalWrite(cs, LOW);
        SPI.transfer(cmd);
        digitalWrite(cs, HIGH);
        delayMicroseconds(2);
        return true;
    }

private:
    const int* _csPins;
    uint8_t _numPins;
};

// --- Setup ---

// Two ADS1299 chips, each with its own CS pin
static const int CS_PINS[] = {10, 9};
MySpiTransport spiTransport(CS_PINS, 2);
Config config(spiTransport, 2);   // 2 ADS1299 chips (16 channels)

void setup() {
    Serial.begin(115200);
    SPI.begin();

    // Board-level settings (applied to all ADCs)
    // Note: daisyEnabled and clkEnabled are managed externally
    // (e.g. by your SPI/streaming driver for daisy-chain readback).
    config.frontends()
        .datarate(ADS1299_DR_250)
        .referenceBufferEnabled(true)
        .biasReferenceInternal(true)
        .biasBufferEnabled(true)
        .continuous(true)
        .leadOffComparatorsEnabled(true);

    // Configure all channels across both chips
    config.channels()
        .enabled(true)
        .gain(ADS1299_PGA_GAIN_24)
        .srb2Closed(true)
        .input(ADS1299_CI_NORMAL)
        .biasPEnable(true)
        .biasNEnable(true);

    // Override channel 0 (chip 0, local ch 0) for test signal
    config.channel(0).input(ADS1299_CI_TEST);

    // Push configuration to hardware
    // This writes each chip's registers via its respective CS pin.
    if (!config.apply()) {
        Serial.print("Config apply failed: ");
        Serial.println(config.lastError());
    }

    // Show register map for both chips
    config.showRegisters(Serial);
}

void loop() {
    // Real-time data streaming (via daisy-chain readback) is handled
    // by a separate API.
}
