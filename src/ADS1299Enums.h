// ADS1299 configuration enums.
// Values correspond to the bit-field encodings in the ADS1299 datasheet.
#pragma once

#include <stdint.h>

namespace ads1299_config {

// Output data rate — CONFIG1 DR[2:0] (Table 13)
// Rates assume fCLK = 2.048 MHz, fMOD = fCLK/2
enum DataRate : uint8_t {
    ADS1299_DR_16000    = 0,  // fMOD/64
    ADS1299_DR_8000     = 1,  // fMOD/128
    ADS1299_DR_4000     = 2,  // fMOD/256
    ADS1299_DR_2000     = 3,  // fMOD/512
    ADS1299_DR_1000     = 4,  // fMOD/1024
    ADS1299_DR_500      = 5,  // fMOD/2048
    ADS1299_DR_250      = 6,  // fMOD/4096
    ADS1299_DR_RESERVED = 7   // Do not use
};

// Test signal frequency — CONFIG2 CAL_FREQ[1:0] (Table 14)
// Approximate values at fCLK = 2.048 MHz
enum TestSignalFrequency : uint8_t {
    ADS1299_TF_1Hz      = 0,  // fCLK/2^21 ≈ 0.977 Hz
    ADS1299_TF_2Hz      = 1,  // fCLK/2^20 ≈ 1.953 Hz
    ADS1299_TF_RESERVED = 2,  // Do not use
    ADS1299_TF_DC       = 3   // DC test signal
};

// Lead-off comparator threshold — LOFF COMP_TH[2:0] (Table 16)
// Positive side threshold (negative side is complementary)
enum ComparatorThreshold : uint8_t {
    ADS1299_CT_950ppt = 0,  // 95.0% positive / 5.0% negative
    ADS1299_CT_925ppt = 1,  // 92.5% / 7.5%
    ADS1299_CT_900ppt = 2,  // 90.0% / 10.0%
    ADS1299_CT_875ppt = 3,  // 87.5% / 12.5%
    ADS1299_CT_850ppt = 4,  // 85.0% / 15.0%
    ADS1299_CT_800ppt = 5,  // 80.0% / 20.0%
    ADS1299_CT_750ppt = 6,  // 75.0% / 25.0%
    ADS1299_CT_700ppt = 7   // 70.0% / 30.0%
};

// Lead-off current magnitude — LOFF ILEAD_OFF[1:0] (Table 16)
enum LeadOffCurrent : uint8_t {
    ADS1299_LOC_6nA  = 0,
    ADS1299_LOC_24nA = 1,
    ADS1299_LOC_6uA  = 2,
    ADS1299_LOC_24uA = 3
};

// Lead-off detection frequency — LOFF FLEAD_OFF[1:0] (Table 16)
// Approximate values at fCLK = 2.048 MHz
enum LeadOffFrequency : uint8_t {
    ADS1299_LOF_DC     = 0,  // DC lead-off detection
    ADS1299_LOF_8Hz    = 1,  // fCLK/2^18 ≈ 7.8 Hz
    ADS1299_LOF_31Hz   = 2,  // fCLK/2^16 ≈ 31.2 Hz
    ADS1299_LOF_fDR_4  = 3   // fDR/4
};

// PGA gain — CHnSET GAINn[2:0] (Table 17)
enum PgaGain : uint8_t {
    ADS1299_PGA_GAIN_1        = 0,
    ADS1299_PGA_GAIN_2        = 1,
    ADS1299_PGA_GAIN_4        = 2,
    ADS1299_PGA_GAIN_6        = 3,
    ADS1299_PGA_GAIN_8        = 4,
    ADS1299_PGA_GAIN_12       = 5,
    ADS1299_PGA_GAIN_24       = 6,
    ADS1299_PGA_GAIN_RESERVED = 7  // Do not use
};

// Channel input multiplexer — CHnSET MUXn[2:0] (Table 17)
enum ChannelInput : uint8_t {
    ADS1299_CI_NORMAL      = 0,  // Normal electrode input
    ADS1299_CI_SHORT       = 1,  // Input shorted (offset/noise measurement)
    ADS1299_CI_BIAS        = 2,  // Used with BIAS_MEAS for BIAS measurements
    ADS1299_CI_MVDD        = 3,  // MVDD for supply measurement
    ADS1299_CI_TEMPERATURE = 4,  // Temperature sensor
    ADS1299_CI_TEST        = 5,  // Test signal
    ADS1299_CI_BIAS_DRP    = 6,  // BIAS_DRP (positive electrode is driver)
    ADS1299_CI_BIAS_DRN    = 7   // BIAS_DRN (negative electrode is driver)
};

} // namespace ads1299_config
