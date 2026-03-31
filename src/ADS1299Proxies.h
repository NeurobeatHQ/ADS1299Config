// Proxy classes for the ADS1299 configuration library.
// Lightweight value types that hold a reference to Config (or RegisterMap)
// and an index. They provide a fluent API at four abstraction levels:
//   FrontendsProxy  — broadcast across all ADC chips
//   FrontendProxy   — single ADC chip
//   ChannelsProxy   — broadcast across all channels on all chips
//   ChannelProxy    — single channel on one chip
//   GpioProxy       — single GPIO pin on one chip
//
// daisyEnabled and clkEnabled are managed externally (not by this API).
// Daisy chaining is used for continuous data readback, not register config.
#pragma once

#include "ADS1299RegisterMap.h"
#include "ADS1299Enums.h"

namespace ads1299_config {

class Config;  // forward declaration

// ============================================================================
// GpioProxy — single GPIO pin (0-3) on one ADS1299 frontend
// ============================================================================

class GpioProxy {
public:
    GpioProxy(RegisterMap& map, uint8_t pin);  // Clamps pin to 0-3

    // Set GPIO direction. — GPIO register GPIOC bit (Table 25)
    // true = input, false = output
    GpioProxy& isInput(bool v);

    // Read GPIO data value. — GPIO register GPIOD bit (Table 25)
    bool value() const;

    // Set GPIO data value (only effective when pin is output).
    // — GPIO register GPIOD bit (Table 25)
    GpioProxy& value(bool v);

private:
    RegisterMap& _map;
    uint8_t _pin;  // 0-3
};

// ============================================================================
// ChannelProxy — single channel on one ADS1299 frontend
// ============================================================================

class ChannelProxy {
public:
    ChannelProxy(RegisterMap& map, uint8_t channelIndex);

    // Enable/disable channel. — CHnSET bit [7] PDn (Table 17)
    // true = normal operation (PDn=0), false = power-down (PDn=1)
    ChannelProxy& enabled(bool v);

    // Set PGA gain. — CHnSET bits [6:4] GAINn (Table 17)
    ChannelProxy& gain(PgaGain v);

    // Set SRB2 connection. — CHnSET bit [3] SRB2 (Table 17)
    // true = closed, false = open
    ChannelProxy& srb2Closed(bool v);

    // Set channel input multiplexer. — CHnSET bits [2:0] MUXn (Table 17)
    ChannelProxy& input(ChannelInput v);

    // Enable positive signal for BIAS derivation. — BIAS_SENSP bit N (Table 18)
    ChannelProxy& biasPEnable(bool v);

    // Enable negative signal for BIAS derivation. — BIAS_SENSN bit N (Table 19)
    ChannelProxy& biasNEnable(bool v);

    // Enable positive signal lead-off detection. — LOFF_SENSP bit N (Table 20)
    ChannelProxy& leadOffPEnable(bool v);

    // Enable negative signal lead-off detection. — LOFF_SENSN bit N (Table 21)
    ChannelProxy& leadOffNEnable(bool v);

    // Set lead-off current polarity flip. — LOFF_FLIP bit N (Table 22)
    // true = flipped, false = normal
    ChannelProxy& leadOffFlip(bool v);

    // Read positive lead-off status. — LOFF_STATP bit N (Table 23, read-only)
    bool leadOffP() const;

    // Read negative lead-off status. — LOFF_STATN bit N (Table 24, read-only)
    bool leadOffN() const;

private:
    void setBit(uint8_t reg, bool v);
    bool getBit(uint8_t reg) const;

    RegisterMap& _map;
    uint8_t _channelIndex;  // 0-based local channel within this frontend
    uint8_t _chSetReg;      // CHnSET register address for this channel
    uint8_t _bitMask;       // Bit mask for per-channel registers (1 << _channelIndex)
};

// ============================================================================
// ChannelsProxy — broadcast across all channels on all frontends
// ============================================================================

class ChannelsProxy {
public:
    ChannelsProxy(Config& config);

    // Access a single channel by global index (0-based across all frontends).
    ChannelProxy channel(uint16_t globalIndex);

    // --- Broadcast setters (apply to all channels) ---

    // Enable/disable all channels. — CHnSET bit [7] PDn (Table 17)
    ChannelsProxy& enabled(bool v);

    // Set PGA gain on all channels. — CHnSET bits [6:4] GAINn (Table 17)
    ChannelsProxy& gain(PgaGain v);

    // Set SRB2 connection on all channels. — CHnSET bit [3] SRB2 (Table 17)
    ChannelsProxy& srb2Closed(bool v);

    // Set channel input on all channels. — CHnSET bits [2:0] MUXn (Table 17)
    ChannelsProxy& input(ChannelInput v);

    // Enable positive BIAS derivation on all channels. — BIAS_SENSP (Table 18)
    ChannelsProxy& biasPEnable(bool v);

    // Enable negative BIAS derivation on all channels. — BIAS_SENSN (Table 19)
    ChannelsProxy& biasNEnable(bool v);

    // Enable positive lead-off detection on all channels. — LOFF_SENSP (Table 20)
    ChannelsProxy& leadOffPEnable(bool v);

    // Enable negative lead-off detection on all channels. — LOFF_SENSN (Table 21)
    ChannelsProxy& leadOffNEnable(bool v);

    // Set lead-off flip on all channels. — LOFF_FLIP (Table 22)
    ChannelsProxy& leadOffFlip(bool v);

private:
    Config& _config;
};

// ============================================================================
// FrontendProxy — single ADS1299 frontend (one chip)
// ============================================================================

class FrontendProxy {
public:
    FrontendProxy(Config& config, uint8_t index);

    // --- Read-only accessors ---

    // Full ID register value. — ID register (0x00, Table 12)
    uint8_t id() const;

    // Device identification. — ID register DEV_ID[3:2] (Table 12)
    uint8_t devId() const;

    // Number of channels on this chip (4, 6, or 8). — ID register NU_CH[1:0] (Table 12)
    uint8_t channelsNumber() const;

    // Positive lead-off status bitmask. — LOFF_STATP register (0x12, Table 23, read-only)
    uint8_t leadOffP() const;

    // Negative lead-off status bitmask. — LOFF_STATN register (0x13, Table 24, read-only)
    uint8_t leadOffN() const;

    // --- CONFIG2 setters ---

    // Set test signal source. — CONFIG2 bit [4] INT_CAL (Table 14)
    // true = internal, false = external
    FrontendProxy& testSourceInternal(bool v);

    // Set test signal amplitude. — CONFIG2 bit [2] CAL_AMP (Table 14)
    // true = 2x amplitude, false = 1x
    FrontendProxy& testSignal2x(bool v);

    // Set test signal frequency. — CONFIG2 bits [1:0] CAL_FREQ (Table 14)
    FrontendProxy& testSignalFrequency(TestSignalFrequency v);

    // --- CONFIG3 setters ---

    // Enable internal reference buffer. — CONFIG3 bit [7] PD_REFBUF (Table 15)
    // true = enabled (PD_REFBUF=1), false = powered down (PD_REFBUF=0)
    FrontendProxy& referenceBufferEnabled(bool v);

    // Enable BIAS measurement. — CONFIG3 bit [4] BIAS_MEAS (Table 15)
    FrontendProxy& biasMeasurementEnabled(bool v);

    // Set BIAS reference signal source. — CONFIG3 bit [3] BIASREF_INT (Table 15)
    // true = internal (AVDD+AVSS)/2, false = external
    FrontendProxy& biasReferenceInternal(bool v);

    // Enable BIAS buffer. — CONFIG3 bit [2] PD_BIAS (Table 15)
    // true = enabled (PD_BIAS=1), false = powered down
    FrontendProxy& biasBufferEnabled(bool v);

    // Enable BIAS sense (lead-off detection on BIAS). — CONFIG3 bit [1] BIAS_LOFF_SENS (Table 15)
    FrontendProxy& biasSenseLeadOffEnabled(bool v);

    // Read BIAS lead-off status. — CONFIG3 bit [0] BIAS_STAT (Table 15, read-only)
    // true = BIAS is not connected, false = BIAS is connected
    bool biasLeadOffNotConnected() const;

    // --- LOFF setters ---

    // Set lead-off comparator threshold. — LOFF bits [7:5] COMP_TH (Table 16)
    FrontendProxy& leadOffComparatorThreshold(ComparatorThreshold v);

    // Set lead-off current magnitude. — LOFF bits [3:2] ILEAD_OFF (Table 16)
    FrontendProxy& leadOffCurrent(LeadOffCurrent v);

    // Set lead-off detection frequency. — LOFF bits [1:0] FLEAD_OFF (Table 16)
    FrontendProxy& leadOffFrequency(LeadOffFrequency v);

    // --- MISC1 setters ---

    // Set SRB1 connection to all inverting inputs. — MISC1 bit [5] SRB1 (Table 26)
    // true = closed, false = open
    FrontendProxy& srb1Closed(bool v);

    // --- CONFIG4 setters ---

    // Set continuous conversion mode. — CONFIG4 bit [3] SINGLE_SHOT (Table 28)
    // true = continuous (SINGLE_SHOT=0), false = single-shot (SINGLE_SHOT=1)
    FrontendProxy& continuous(bool v);

    // Enable lead-off comparators. — CONFIG4 bit [1] PD_LOFF_COMP (Table 28)
    // true = enabled (PD_LOFF_COMP=1), false = disabled
    FrontendProxy& leadOffComparatorsEnabled(bool v);

    // --- GPIO ---

    // Access a GPIO pin (0-3) on this frontend. — GPIO register (0x14, Table 25)
    GpioProxy gpio(uint8_t pin);

    // --- Channel access ---

    // Access a channel by local index (0-based within this frontend).
    ChannelProxy channel(uint8_t localIndex);

    // --- Legacy register access ---

    // Write a raw register value. Bypasses fixed-bit enforcement from setField
    // but enforceFixedBits is still called by RegisterMap::write.
    void wreg(uint8_t reg, uint8_t value);

    // Read a raw register value from the shadow map.
    uint8_t rreg(uint8_t reg) const;

private:
    RegisterMap& map();
    const RegisterMap& map() const;

    Config& _config;
    uint8_t _index;
};

// ============================================================================
// FrontendsProxy — broadcast across all ADS1299 frontends
// ============================================================================

class FrontendsProxy {
public:
    FrontendsProxy(Config& config);

    // --- Board-level CONFIG1 setter ---

    // Set output data rate. — CONFIG1 bits [2:0] DR (Table 13)
    // Applied identically to all ADCs.
    FrontendsProxy& datarate(DataRate v);

    // --- Per-ADC setters (broadcast to all) ---

    // Set test signal source. — CONFIG2 bit [4] INT_CAL (Table 14)
    FrontendsProxy& testSourceInternal(bool v);

    // Set test signal amplitude. — CONFIG2 bit [2] CAL_AMP (Table 14)
    FrontendsProxy& testSignal2x(bool v);

    // Set test signal frequency. — CONFIG2 bits [1:0] CAL_FREQ (Table 14)
    FrontendsProxy& testSignalFrequency(TestSignalFrequency v);

    // Enable internal reference buffer. — CONFIG3 bit [7] PD_REFBUF (Table 15)
    FrontendsProxy& referenceBufferEnabled(bool v);

    // Enable BIAS measurement. — CONFIG3 bit [4] BIAS_MEAS (Table 15)
    FrontendsProxy& biasMeasurementEnabled(bool v);

    // Set BIAS reference source. — CONFIG3 bit [3] BIASREF_INT (Table 15)
    FrontendsProxy& biasReferenceInternal(bool v);

    // Enable BIAS buffer. — CONFIG3 bit [2] PD_BIAS (Table 15)
    FrontendsProxy& biasBufferEnabled(bool v);

    // Enable BIAS sense lead-off. — CONFIG3 bit [1] BIAS_LOFF_SENS (Table 15)
    FrontendsProxy& biasSenseLeadOffEnabled(bool v);

    // Set lead-off comparator threshold. — LOFF bits [7:5] COMP_TH (Table 16)
    FrontendsProxy& leadOffComparatorThreshold(ComparatorThreshold v);

    // Set lead-off current magnitude. — LOFF bits [3:2] ILEAD_OFF (Table 16)
    FrontendsProxy& leadOffCurrent(LeadOffCurrent v);

    // Set lead-off detection frequency. — LOFF bits [1:0] FLEAD_OFF (Table 16)
    FrontendsProxy& leadOffFrequency(LeadOffFrequency v);

    // Set SRB1 connection. — MISC1 bit [5] SRB1 (Table 26)
    FrontendsProxy& srb1Closed(bool v);

    // Set continuous mode. — CONFIG4 bit [3] SINGLE_SHOT (Table 28)
    FrontendsProxy& continuous(bool v);

    // Enable lead-off comparators. — CONFIG4 bit [1] PD_LOFF_COMP (Table 28)
    FrontendsProxy& leadOffComparatorsEnabled(bool v);

    // --- Read accessors (aggregated across all frontends) ---

    // Fill buffer with positive lead-off status bytes, one per frontend.
    // buf[0] = frontend 0 LOFF_STATP, buf[1] = frontend 1, etc.
    // Returns number of bytes written (min(numFrontends, len)).
    // — LOFF_STATP registers (Table 23, read-only)
    uint8_t leadOffP(uint8_t* buf, uint8_t len) const;

    // Fill buffer with negative lead-off status bytes, one per frontend.
    // — LOFF_STATN registers (Table 24, read-only)
    uint8_t leadOffN(uint8_t* buf, uint8_t len) const;

    // --- Channel access ---

    // Access a single channel by global index.
    ChannelProxy channel(uint16_t globalIndex);

    // Access all channels as a broadcast proxy.
    ChannelsProxy channels();

private:
    Config& _config;
};

} // namespace ads1299_config
