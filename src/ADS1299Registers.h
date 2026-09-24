// ADS1299 Register addresses, bit masks, shifts, fixed bits, and defaults.
// Reference: ADS1299 datasheet SBAS499C, Table 11 (Register Assignments)
#pragma once

#include <cstdint>

namespace ads1299_config {
namespace reg {

// Register addresses (Table 11)
constexpr uint8_t ID          = 0x00;  // ID Control Register (read-only)
constexpr uint8_t CONFIG1     = 0x01;  // Configuration Register 1
constexpr uint8_t CONFIG2     = 0x02;  // Configuration Register 2
constexpr uint8_t CONFIG3     = 0x03;  // Configuration Register 3
constexpr uint8_t LOFF        = 0x04;  // Lead-Off Control Register
constexpr uint8_t CH1SET      = 0x05;  // Channel 1 Settings
constexpr uint8_t CH2SET      = 0x06;  // Channel 2 Settings
constexpr uint8_t CH3SET      = 0x07;  // Channel 3 Settings
constexpr uint8_t CH4SET      = 0x08;  // Channel 4 Settings
constexpr uint8_t CH5SET      = 0x09;  // Channel 5 Settings
constexpr uint8_t CH6SET      = 0x0A;  // Channel 6 Settings
constexpr uint8_t CH7SET      = 0x0B;  // Channel 7 Settings
constexpr uint8_t CH8SET      = 0x0C;  // Channel 8 Settings
constexpr uint8_t BIAS_SENSP  = 0x0D;  // Bias Drive Positive Derivation
constexpr uint8_t BIAS_SENSN  = 0x0E;  // Bias Drive Negative Derivation
constexpr uint8_t LOFF_SENSP  = 0x0F;  // Positive Signal Lead-Off Detection
constexpr uint8_t LOFF_SENSN  = 0x10;  // Negative Signal Lead-Off Detection
constexpr uint8_t LOFF_FLIP   = 0x11;  // Lead-Off Flip
constexpr uint8_t LOFF_STATP  = 0x12;  // Lead-Off Positive Status (read-only)
constexpr uint8_t LOFF_STATN  = 0x13;  // Lead-Off Negative Status (read-only)
constexpr uint8_t GPIO        = 0x14;  // General-Purpose I/O
constexpr uint8_t MISC1       = 0x15;  // Miscellaneous 1
constexpr uint8_t MISC2       = 0x16;  // Miscellaneous 2 (reserved)
constexpr uint8_t CONFIG4     = 0x17;  // Configuration Register 4

constexpr uint8_t NUM_REGISTERS = 0x18;  // 24 registers total

// --- Register defaults (Table 11) ---
constexpr uint8_t CONFIG1_DEFAULT  = 0x96;  // 1001 0110
constexpr uint8_t CONFIG2_DEFAULT  = 0xC0;  // 1100 0000
constexpr uint8_t CONFIG3_DEFAULT  = 0x60;  // 0110 0000
constexpr uint8_t LOFF_DEFAULT     = 0x00;
constexpr uint8_t CHNSET_DEFAULT   = 0x61;  // 0110 0001
constexpr uint8_t GPIO_DEFAULT     = 0x0F;  // 0000 1111

// --- Fixed bit masks (bits that must always be written as specific values) ---
// CONFIG1: bit7=1, bits[4:3]=10
constexpr uint8_t CONFIG1_FIXED_MASK  = 0x98;  // 1001 1000
constexpr uint8_t CONFIG1_FIXED_VALUE = 0x90;  // 1001 0000

// CONFIG2: bits[7:5]=110, bit3=0
constexpr uint8_t CONFIG2_FIXED_MASK  = 0xE8;  // 1110 1000
constexpr uint8_t CONFIG2_FIXED_VALUE = 0xC0;  // 1100 0000

// CONFIG3: bits[6:5]=11
constexpr uint8_t CONFIG3_FIXED_MASK  = 0x60;  // 0110 0000
constexpr uint8_t CONFIG3_FIXED_VALUE = 0x60;  // 0110 0000

// LOFF: bit4=0
constexpr uint8_t LOFF_FIXED_MASK  = 0x10;  // 0001 0000
constexpr uint8_t LOFF_FIXED_VALUE = 0x00;

// --- ID Register (0x00) fields ---
constexpr uint8_t ID_REV_ID_MASK   = 0xE0;  // bits [7:5]
constexpr uint8_t ID_REV_ID_SHIFT  = 5;
constexpr uint8_t ID_DEV_ID_MASK   = 0x0C;  // bits [3:2]
constexpr uint8_t ID_DEV_ID_SHIFT  = 2;
constexpr uint8_t ID_NU_CH_MASK    = 0x03;  // bits [1:0]
constexpr uint8_t ID_NU_CH_SHIFT   = 0;

// --- CONFIG1 (0x01) fields ---
// DAISY_EN: bit 6 — 0=daisy-chain, 1=multiple readback
constexpr uint8_t CONFIG1_DAISY_EN_MASK  = 0x40;
constexpr uint8_t CONFIG1_DAISY_EN_SHIFT = 6;
// CLK_EN: bit 5 — 0=osc clock output disabled, 1=enabled
constexpr uint8_t CONFIG1_CLK_EN_MASK    = 0x20;
constexpr uint8_t CONFIG1_CLK_EN_SHIFT   = 5;
// DR[2:0]: bits [2:0] — output data rate
constexpr uint8_t CONFIG1_DR_MASK        = 0x07;
constexpr uint8_t CONFIG1_DR_SHIFT       = 0;

// --- CONFIG2 (0x02) fields ---
// INT_CAL: bit 4 — 0=external test source, 1=internal
constexpr uint8_t CONFIG2_INT_CAL_MASK    = 0x10;
constexpr uint8_t CONFIG2_INT_CAL_SHIFT   = 4;
// CAL_AMP: bit 2 — 0=1x amplitude, 1=2x amplitude
constexpr uint8_t CONFIG2_CAL_AMP_MASK    = 0x04;
constexpr uint8_t CONFIG2_CAL_AMP_SHIFT   = 2;
// CAL_FREQ[1:0]: bits [1:0] — test signal frequency
constexpr uint8_t CONFIG2_CAL_FREQ_MASK   = 0x03;
constexpr uint8_t CONFIG2_CAL_FREQ_SHIFT  = 0;

// --- CONFIG3 (0x03) fields ---
// PD_REFBUF: bit 7 — 0=power-down ref buffer, 1=enable
constexpr uint8_t CONFIG3_PD_REFBUF_MASK       = 0x80;
constexpr uint8_t CONFIG3_PD_REFBUF_SHIFT      = 7;
// BIAS_MEAS: bit 4 — 0=open, 1=BIAS_IN routed to MUX 010
constexpr uint8_t CONFIG3_BIAS_MEAS_MASK       = 0x10;
constexpr uint8_t CONFIG3_BIAS_MEAS_SHIFT      = 4;
// BIASREF_INT: bit 3 — 0=external BIASREF, 1=internal (AVDD+AVSS)/2
constexpr uint8_t CONFIG3_BIASREF_INT_MASK     = 0x08;
constexpr uint8_t CONFIG3_BIASREF_INT_SHIFT    = 3;
// PD_BIAS: bit 2 — 0=BIAS buffer powered down, 1=enabled
constexpr uint8_t CONFIG3_PD_BIAS_MASK         = 0x04;
constexpr uint8_t CONFIG3_PD_BIAS_SHIFT        = 2;
// BIAS_LOFF_SENS: bit 1 — 0=BIAS sense disabled, 1=enabled
constexpr uint8_t CONFIG3_BIAS_LOFF_SENS_MASK  = 0x02;
constexpr uint8_t CONFIG3_BIAS_LOFF_SENS_SHIFT = 1;
// BIAS_STAT: bit 0 — read-only, 0=connected, 1=not connected
constexpr uint8_t CONFIG3_BIAS_STAT_MASK       = 0x01;
constexpr uint8_t CONFIG3_BIAS_STAT_SHIFT      = 0;

// --- LOFF (0x04) fields ---
// COMP_TH[2:0]: bits [7:5] — comparator threshold
constexpr uint8_t LOFF_COMP_TH_MASK       = 0xE0;
constexpr uint8_t LOFF_COMP_TH_SHIFT      = 5;
// ILEAD_OFF[1:0]: bits [3:2] — lead-off current magnitude
constexpr uint8_t LOFF_ILEAD_OFF_MASK     = 0x0C;
constexpr uint8_t LOFF_ILEAD_OFF_SHIFT    = 2;
// FLEAD_OFF[1:0]: bits [1:0] — lead-off frequency
constexpr uint8_t LOFF_FLEAD_OFF_MASK     = 0x03;
constexpr uint8_t LOFF_FLEAD_OFF_SHIFT    = 0;

// --- CHnSET (0x05-0x0C) fields ---
// PDn: bit 7 — 0=normal, 1=channel power-down
constexpr uint8_t CHNSET_PD_MASK     = 0x80;
constexpr uint8_t CHNSET_PD_SHIFT    = 7;
// GAINn[2:0]: bits [6:4] — PGA gain
constexpr uint8_t CHNSET_GAIN_MASK   = 0x70;
constexpr uint8_t CHNSET_GAIN_SHIFT  = 4;
// SRB2: bit 3 — 0=open, 1=closed
constexpr uint8_t CHNSET_SRB2_MASK   = 0x08;
constexpr uint8_t CHNSET_SRB2_SHIFT  = 3;
// MUXn[2:0]: bits [2:0] — channel input selection
constexpr uint8_t CHNSET_MUX_MASK    = 0x07;
constexpr uint8_t CHNSET_MUX_SHIFT   = 0;

// --- GPIO (0x14) fields ---
// GPIOD[4:1]: bits [7:4] — GPIO data
constexpr uint8_t GPIO_GPIOD_MASK   = 0xF0;
constexpr uint8_t GPIO_GPIOD_SHIFT  = 4;
// GPIOC[4:1]: bits [3:0] — GPIO control (0=output, 1=input)
constexpr uint8_t GPIO_GPIOC_MASK   = 0x0F;
constexpr uint8_t GPIO_GPIOC_SHIFT  = 0;

// --- MISC1 (0x15) fields ---
// SRB1: bit 5 — 0=open, 1=closed (routes SRB1 to all inverting inputs)
constexpr uint8_t MISC1_SRB1_MASK   = 0x20;
constexpr uint8_t MISC1_SRB1_SHIFT  = 5;

// --- CONFIG4 (0x17) fields ---
// SINGLE_SHOT: bit 3 — 0=continuous, 1=single-shot
constexpr uint8_t CONFIG4_SINGLE_SHOT_MASK    = 0x08;
constexpr uint8_t CONFIG4_SINGLE_SHOT_SHIFT   = 3;
// PD_LOFF_COMP: bit 1 — 0=lead-off comparators disabled, 1=enabled
constexpr uint8_t CONFIG4_PD_LOFF_COMP_MASK   = 0x02;
constexpr uint8_t CONFIG4_PD_LOFF_COMP_SHIFT  = 1;
// CONFIG4 fixed: bits[7:4]=0, bit2=0, bit0=0
constexpr uint8_t CONFIG4_FIXED_MASK  = 0xF5;  // 1111 0101
constexpr uint8_t CONFIG4_FIXED_VALUE = 0x00;

// --- SPI Commands (Table 10) ---
constexpr uint8_t CMD_WAKEUP  = 0x02;
constexpr uint8_t CMD_STANDBY = 0x04;
constexpr uint8_t CMD_RESET   = 0x06;
constexpr uint8_t CMD_START   = 0x08;
constexpr uint8_t CMD_STOP    = 0x0A;
constexpr uint8_t CMD_RDATAC  = 0x10;
constexpr uint8_t CMD_SDATAC  = 0x11;
constexpr uint8_t CMD_RDATA   = 0x12;

// --- Helpers ---

// Returns true if the register is read-only (ID, LOFF_STATP, LOFF_STATN).
constexpr bool isReadOnly(uint8_t r) {
    return r == ID || r == LOFF_STATP || r == LOFF_STATN;
}

} // namespace reg
} // namespace ads1299_config
