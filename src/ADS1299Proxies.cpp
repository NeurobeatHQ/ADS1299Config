#include "ADS1299Proxies.h"
#include "ADS1299Config.h"

namespace ads1299_config {

// ============================================================================
// GpioProxy
// ============================================================================

GpioProxy::GpioProxy(RegisterMap& map, uint8_t pin)
    : _map(map), _pin(pin & 0x03) {}

GpioProxy& GpioProxy::isInput(bool v) {
    uint8_t val = _map.read(reg::GPIO);
    uint8_t bit = 1 << _pin;  // GPIOC bits [3:0]
    if (v) val |= bit;
    else   val &= ~bit;
    _map.write(reg::GPIO, val);
    return *this;
}

bool GpioProxy::value() const {
    uint8_t val = _map.read(reg::GPIO);
    uint8_t bit = 1 << (_pin + 4);  // GPIOD bits [7:4]
    return (val & bit) != 0;
}

GpioProxy& GpioProxy::value(bool v) {
    uint8_t val = _map.read(reg::GPIO);
    uint8_t bit = 1 << (_pin + 4);  // GPIOD bits [7:4]
    if (v) val |= bit;
    else   val &= ~bit;
    _map.write(reg::GPIO, val);
    return *this;
}

// ============================================================================
// ChannelProxy
// ============================================================================

ChannelProxy::ChannelProxy(RegisterMap& map, uint8_t channelIndex)
    : _map(map)
    , _channelIndex(channelIndex)
    , _chSetReg(reg::CH1SET + channelIndex)
    , _bitMask(1 << channelIndex)
{
}

ChannelProxy& ChannelProxy::enabled(bool v) {
    // CHnSET bit [7] PDn: 0=normal, 1=power-down (inverted logic)
    _map.setField(_chSetReg, reg::CHNSET_PD_MASK, reg::CHNSET_PD_SHIFT, v ? 0 : 1);
    return *this;
}

ChannelProxy& ChannelProxy::gain(PgaGain v) {
    // CHnSET bits [6:4] GAINn
    _map.setField(_chSetReg, reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT, v);
    return *this;
}

ChannelProxy& ChannelProxy::srb2Closed(bool v) {
    // CHnSET bit [3] SRB2: 0=open, 1=closed
    _map.setField(_chSetReg, reg::CHNSET_SRB2_MASK, reg::CHNSET_SRB2_SHIFT, v ? 1 : 0);
    return *this;
}

ChannelProxy& ChannelProxy::input(ChannelInput v) {
    // CHnSET bits [2:0] MUXn
    _map.setField(_chSetReg, reg::CHNSET_MUX_MASK, reg::CHNSET_MUX_SHIFT, v);
    return *this;
}

ChannelProxy& ChannelProxy::biasPEnable(bool v) {
    // BIAS_SENSP bit N
    setBit(reg::BIAS_SENSP, v);
    return *this;
}

ChannelProxy& ChannelProxy::biasNEnable(bool v) {
    // BIAS_SENSN bit N
    setBit(reg::BIAS_SENSN, v);
    return *this;
}

ChannelProxy& ChannelProxy::leadOffPEnable(bool v) {
    // LOFF_SENSP bit N
    setBit(reg::LOFF_SENSP, v);
    return *this;
}

ChannelProxy& ChannelProxy::leadOffNEnable(bool v) {
    // LOFF_SENSN bit N
    setBit(reg::LOFF_SENSN, v);
    return *this;
}

ChannelProxy& ChannelProxy::leadOffFlip(bool v) {
    // LOFF_FLIP bit N
    setBit(reg::LOFF_FLIP, v);
    return *this;
}

bool ChannelProxy::leadOffP() const {
    // LOFF_STATP bit N (read-only)
    return getBit(reg::LOFF_STATP);
}

bool ChannelProxy::leadOffN() const {
    // LOFF_STATN bit N (read-only)
    return getBit(reg::LOFF_STATN);
}

void ChannelProxy::setBit(uint8_t reg, bool v) {
    uint8_t val = _map.read(reg);
    if (v) val |= _bitMask;
    else   val &= ~_bitMask;
    _map.write(reg, val);
}

bool ChannelProxy::getBit(uint8_t reg) const {
    return (_map.read(reg) & _bitMask) != 0;
}

// ============================================================================
// ChannelsProxy
// ============================================================================

ChannelsProxy::ChannelsProxy(Config& config)
    : _config(config) {}

ChannelProxy ChannelsProxy::channel(uint16_t globalIndex) {
    return _config.channel(globalIndex);
}

// Broadcast setters iterate per-frontend then per-local-channel to avoid
// O(channels × frontends) overhead of repeated global-to-local lookups.

ChannelsProxy& ChannelsProxy::enabled(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).enabled(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::gain(PgaGain v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).gain(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::srb2Closed(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).srb2Closed(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::input(ChannelInput v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).input(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::biasPEnable(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).biasPEnable(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::biasNEnable(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).biasNEnable(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::leadOffPEnable(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).leadOffPEnable(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::leadOffNEnable(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).leadOffNEnable(v);
    }
    return *this;
}

ChannelsProxy& ChannelsProxy::leadOffFlip(bool v) {
    for (uint8_t fe = 0; fe < _config.numFrontends(); fe++) {
        uint8_t n = _config.registerMap(fe).numChannels();
        for (uint8_t ch = 0; ch < n; ch++)
            ChannelProxy(_config.registerMap(fe), ch).leadOffFlip(v);
    }
    return *this;
}

// ============================================================================
// FrontendProxy
// ============================================================================

FrontendProxy::FrontendProxy(Config& config, uint8_t index)
    : _config(config), _index(index) {}

RegisterMap& FrontendProxy::map() {
    return _config.registerMap(_index);
}

const RegisterMap& FrontendProxy::map() const {
    return _config.registerMap(_index);
}

// --- Read-only accessors ---

uint8_t FrontendProxy::id() const {
    return map().read(reg::ID);
}

uint8_t FrontendProxy::devId() const {
    return map().getField(reg::ID, reg::ID_DEV_ID_MASK, reg::ID_DEV_ID_SHIFT);
}

uint8_t FrontendProxy::channelsNumber() const {
    return map().numChannels();
}

uint8_t FrontendProxy::leadOffP() const {
    return map().read(reg::LOFF_STATP);
}

uint8_t FrontendProxy::leadOffN() const {
    return map().read(reg::LOFF_STATN);
}

// --- CONFIG2 setters ---

FrontendProxy& FrontendProxy::testSourceInternal(bool v) {
    // CONFIG2 bit [4] INT_CAL: 0=external, 1=internal
    map().setField(reg::CONFIG2, reg::CONFIG2_INT_CAL_MASK, reg::CONFIG2_INT_CAL_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::testSignal2x(bool v) {
    // CONFIG2 bit [2] CAL_AMP: 0=1x, 1=2x
    map().setField(reg::CONFIG2, reg::CONFIG2_CAL_AMP_MASK, reg::CONFIG2_CAL_AMP_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::testSignalFrequency(TestSignalFrequency v) {
    // CONFIG2 bits [1:0] CAL_FREQ
    map().setField(reg::CONFIG2, reg::CONFIG2_CAL_FREQ_MASK, reg::CONFIG2_CAL_FREQ_SHIFT, v);
    return *this;
}

// --- CONFIG3 setters ---

FrontendProxy& FrontendProxy::referenceBufferEnabled(bool v) {
    // CONFIG3 bit [7] PD_REFBUF: 0=power-down, 1=enable
    map().setField(reg::CONFIG3, reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::biasMeasurementEnabled(bool v) {
    // CONFIG3 bit [4] BIAS_MEAS: 0=open, 1=BIAS_IN routed
    map().setField(reg::CONFIG3, reg::CONFIG3_BIAS_MEAS_MASK, reg::CONFIG3_BIAS_MEAS_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::biasReferenceInternal(bool v) {
    // CONFIG3 bit [3] BIASREF_INT: 0=external, 1=internal
    map().setField(reg::CONFIG3, reg::CONFIG3_BIASREF_INT_MASK, reg::CONFIG3_BIASREF_INT_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::biasBufferEnabled(bool v) {
    // CONFIG3 bit [2] PD_BIAS: 0=powered down, 1=enabled
    map().setField(reg::CONFIG3, reg::CONFIG3_PD_BIAS_MASK, reg::CONFIG3_PD_BIAS_SHIFT, v ? 1 : 0);
    return *this;
}

FrontendProxy& FrontendProxy::biasSenseLeadOffEnabled(bool v) {
    // CONFIG3 bit [1] BIAS_LOFF_SENS: 0=disabled, 1=enabled
    map().setField(reg::CONFIG3, reg::CONFIG3_BIAS_LOFF_SENS_MASK, reg::CONFIG3_BIAS_LOFF_SENS_SHIFT, v ? 1 : 0);
    return *this;
}

bool FrontendProxy::biasLeadOffNotConnected() const {
    // CONFIG3 bit [0] BIAS_STAT: 0=connected, 1=not connected (read-only)
    return map().getField(reg::CONFIG3, reg::CONFIG3_BIAS_STAT_MASK, reg::CONFIG3_BIAS_STAT_SHIFT) != 0;
}

// --- LOFF setters ---

FrontendProxy& FrontendProxy::leadOffComparatorThreshold(ComparatorThreshold v) {
    // LOFF bits [7:5] COMP_TH
    map().setField(reg::LOFF, reg::LOFF_COMP_TH_MASK, reg::LOFF_COMP_TH_SHIFT, v);
    return *this;
}

FrontendProxy& FrontendProxy::leadOffCurrent(LeadOffCurrent v) {
    // LOFF bits [3:2] ILEAD_OFF
    map().setField(reg::LOFF, reg::LOFF_ILEAD_OFF_MASK, reg::LOFF_ILEAD_OFF_SHIFT, v);
    return *this;
}

FrontendProxy& FrontendProxy::leadOffFrequency(LeadOffFrequency v) {
    // LOFF bits [1:0] FLEAD_OFF
    map().setField(reg::LOFF, reg::LOFF_FLEAD_OFF_MASK, reg::LOFF_FLEAD_OFF_SHIFT, v);
    return *this;
}

// --- MISC1 setters ---

FrontendProxy& FrontendProxy::srb1Closed(bool v) {
    // MISC1 bit [5] SRB1: 0=open, 1=closed
    map().setField(reg::MISC1, reg::MISC1_SRB1_MASK, reg::MISC1_SRB1_SHIFT, v ? 1 : 0);
    return *this;
}

// --- CONFIG4 setters ---

FrontendProxy& FrontendProxy::continuous(bool v) {
    // CONFIG4 bit [3] SINGLE_SHOT: 0=continuous, 1=single-shot (inverted)
    map().setField(reg::CONFIG4, reg::CONFIG4_SINGLE_SHOT_MASK, reg::CONFIG4_SINGLE_SHOT_SHIFT, v ? 0 : 1);
    return *this;
}

FrontendProxy& FrontendProxy::leadOffComparatorsEnabled(bool v) {
    // CONFIG4 bit [1] PD_LOFF_COMP: 0=disabled, 1=enabled
    map().setField(reg::CONFIG4, reg::CONFIG4_PD_LOFF_COMP_MASK, reg::CONFIG4_PD_LOFF_COMP_SHIFT, v ? 1 : 0);
    return *this;
}

// --- GPIO ---

GpioProxy FrontendProxy::gpio(uint8_t pin) {
    return GpioProxy(map(), pin);
}

// --- Channel access ---

ChannelProxy FrontendProxy::channel(uint8_t localIndex) {
    return ChannelProxy(map(), localIndex);
}

// --- Legacy ---

void FrontendProxy::wreg(uint8_t reg, uint8_t value) {
    map().write(reg, value);
}

uint8_t FrontendProxy::rreg(uint8_t reg) const {
    return map().read(reg);
}

// ============================================================================
// FrontendsProxy
// ============================================================================

FrontendsProxy::FrontendsProxy(Config& config)
    : _config(config) {}

// --- Board-level CONFIG1 setter ---

FrontendsProxy& FrontendsProxy::datarate(DataRate v) {
    // CONFIG1 bits [2:0] DR — same value on all ADCs
    for (uint8_t i = 0; i < _config.numFrontends(); i++) {
        _config.registerMap(i).setField(reg::CONFIG1,
            reg::CONFIG1_DR_MASK, reg::CONFIG1_DR_SHIFT, v);
    }
    return *this;
}

// --- Broadcast per-ADC setters ---

FrontendsProxy& FrontendsProxy::testSourceInternal(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).testSourceInternal(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::testSignal2x(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).testSignal2x(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::testSignalFrequency(TestSignalFrequency v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).testSignalFrequency(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::referenceBufferEnabled(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).referenceBufferEnabled(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::biasMeasurementEnabled(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).biasMeasurementEnabled(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::biasReferenceInternal(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).biasReferenceInternal(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::biasBufferEnabled(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).biasBufferEnabled(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::biasSenseLeadOffEnabled(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).biasSenseLeadOffEnabled(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::leadOffComparatorThreshold(ComparatorThreshold v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).leadOffComparatorThreshold(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::leadOffCurrent(LeadOffCurrent v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).leadOffCurrent(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::leadOffFrequency(LeadOffFrequency v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).leadOffFrequency(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::srb1Closed(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).srb1Closed(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::continuous(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).continuous(v);
    return *this;
}

FrontendsProxy& FrontendsProxy::leadOffComparatorsEnabled(bool v) {
    for (uint8_t i = 0; i < _config.numFrontends(); i++)
        _config.frontend(i).leadOffComparatorsEnabled(v);
    return *this;
}

// --- Read accessors ---

uint8_t FrontendsProxy::leadOffP(uint8_t* buf, uint8_t len) const {
    uint8_t count = _config.numFrontends();
    if (count > len) count = len;
    for (uint8_t i = 0; i < count; i++) {
        buf[i] = _config.registerMap(i).read(reg::LOFF_STATP);
    }
    return count;
}

uint8_t FrontendsProxy::leadOffN(uint8_t* buf, uint8_t len) const {
    uint8_t count = _config.numFrontends();
    if (count > len) count = len;
    for (uint8_t i = 0; i < count; i++) {
        buf[i] = _config.registerMap(i).read(reg::LOFF_STATN);
    }
    return count;
}

// --- Channel access ---

ChannelProxy FrontendsProxy::channel(uint16_t globalIndex) {
    return _config.channel(globalIndex);
}

ChannelsProxy FrontendsProxy::channels() {
    return _config.channels();
}

} // namespace ads1299_config
