#include "ADS1299RegisterMap.h"

namespace ads1299_config {

RegisterMap::RegisterMap() {
    resetDefaults();
}

void RegisterMap::resetDefaults(uint8_t numChannels) {
    // Zero everything first
    for (uint8_t i = 0; i < reg::NUM_REGISTERS; i++) {
        _regs[i] = 0x00;
        _dirty[i] = false;
    }

    // Set ID register NU_CH bits so numChannels() returns the correct count
    // even before load() reads the actual hardware ID register.
    // NU_CH encoding: 00=4ch, 01=6ch, 10=8ch
    uint8_t nu_ch = (numChannels >= 8) ? 2 : (numChannels >= 6) ? 1 : 0;
    _regs[reg::ID] = (nu_ch << reg::ID_NU_CH_SHIFT) & reg::ID_NU_CH_MASK;

    // Set datasheet defaults (Table 11)
    _regs[reg::CONFIG1] = reg::CONFIG1_DEFAULT;  // 0x96
    _regs[reg::CONFIG2] = reg::CONFIG2_DEFAULT;  // 0xC0
    _regs[reg::CONFIG3] = reg::CONFIG3_DEFAULT;  // 0x60

    // CHnSET registers default to 0x61
    for (uint8_t i = reg::CH1SET; i <= reg::CH8SET; i++) {
        _regs[i] = reg::CHNSET_DEFAULT;
    }

    // GPIO defaults to 0x0F (all pins input)
    _regs[reg::GPIO] = reg::GPIO_DEFAULT;
}

void RegisterMap::setField(uint8_t reg, uint8_t mask, uint8_t shift, uint8_t value) {
    if (reg >= reg::NUM_REGISTERS) return;
    _regs[reg] = (_regs[reg] & ~mask) | ((value << shift) & mask);
    enforceFixedBits(reg);
    _dirty[reg] = true;
}

uint8_t RegisterMap::getField(uint8_t reg, uint8_t mask, uint8_t shift) const {
    if (reg >= reg::NUM_REGISTERS) return 0;
    return (_regs[reg] & mask) >> shift;
}

uint8_t RegisterMap::read(uint8_t reg) const {
    if (reg >= reg::NUM_REGISTERS) return 0;
    return _regs[reg];
}

void RegisterMap::write(uint8_t reg, uint8_t value) {
    if (reg >= reg::NUM_REGISTERS) return;
    _regs[reg] = value;
    enforceFixedBits(reg);
    _dirty[reg] = true;
}

bool RegisterMap::isDirty(uint8_t reg) const {
    if (reg >= reg::NUM_REGISTERS) return false;
    return _dirty[reg];
}

void RegisterMap::clearDirty(uint8_t reg) {
    if (reg >= reg::NUM_REGISTERS) return;
    _dirty[reg] = false;
}

void RegisterMap::clearAllDirty() {
    for (uint8_t i = 0; i < reg::NUM_REGISTERS; i++) {
        _dirty[i] = false;
    }
}

uint8_t RegisterMap::numChannels() const {
    // NU_CH[1:0] in ID register: 00=4, 01=6, 10=8
    uint8_t nu_ch = getField(reg::ID, reg::ID_NU_CH_MASK, reg::ID_NU_CH_SHIFT);
    switch (nu_ch) {
        case 0: return 4;
        case 1: return 6;
        case 2: return 8;
        default: return 8;  // Assume 8 for unknown
    }
}

void RegisterMap::enforceFixedBits(uint8_t reg) {
    switch (reg) {
        case reg::CONFIG1:
            _regs[reg] = (_regs[reg] & ~reg::CONFIG1_FIXED_MASK) | reg::CONFIG1_FIXED_VALUE;
            break;
        case reg::CONFIG2:
            _regs[reg] = (_regs[reg] & ~reg::CONFIG2_FIXED_MASK) | reg::CONFIG2_FIXED_VALUE;
            break;
        case reg::CONFIG3:
            _regs[reg] = (_regs[reg] & ~reg::CONFIG3_FIXED_MASK) | reg::CONFIG3_FIXED_VALUE;
            break;
        case reg::LOFF:
            _regs[reg] = (_regs[reg] & ~reg::LOFF_FIXED_MASK) | reg::LOFF_FIXED_VALUE;
            break;
        case reg::CONFIG4:
            _regs[reg] = (_regs[reg] & ~reg::CONFIG4_FIXED_MASK) | reg::CONFIG4_FIXED_VALUE;
            break;
        default:
            break;
    }
}

} // namespace ads1299_config
