// Desktop test for ADS1299Config library.
// Compile: g++ -std=c++11 -I../src test_config.cpp ../src/ADS1299RegisterMap.cpp ../src/ADS1299Proxies.cpp ../src/ADS1299Config.cpp -o test_config
#include <cassert>
#include <cstdio>
#include <cstring>
#include "ADS1299Config.h"

using namespace ads1299_config;

// --- Mock transport ---
class MockTransport : public Transport {
public:
    static constexpr int MAX_FE = 128;
    uint8_t regs[MAX_FE][reg::NUM_REGISTERS];
    int writeCount;
    int readCount;
    int cmdCount;
    bool failNext;

    MockTransport() : writeCount(0), readCount(0), cmdCount(0), failNext(false) {
        memset(regs, 0, sizeof(regs));
    }

    bool writeRegister(uint8_t fe, uint8_t reg, uint8_t value) override {
        if (failNext) { failNext = false; return false; }
        if (fe < MAX_FE) regs[fe][reg] = value;
        writeCount++;
        return true;
    }

    bool readRegister(uint8_t fe, uint8_t reg, uint8_t& value) override {
        if (failNext) { failNext = false; return false; }
        value = (fe < MAX_FE) ? regs[fe][reg] : 0;
        readCount++;
        return true;
    }

    bool sendCommand(uint8_t fe, uint8_t cmd) override {
        if (failNext) { failNext = false; return false; }
        cmdCount++;
        return true;
    }

    void reset() {
        memset(regs, 0, sizeof(regs));
        writeCount = readCount = cmdCount = 0;
        failNext = false;
    }
};

// --- Print shim that captures output ---
class StringPrint : public Print {
public:
    char buf[8192];
    size_t pos;

    StringPrint() : pos(0) { buf[0] = 0; }

    size_t write(uint8_t c) override {
        if (pos < sizeof(buf) - 1) {
            buf[pos++] = (char)c;
            buf[pos] = 0;
        }
        return 1;
    }
};

void test_register_map_defaults() {
    printf("test_register_map_defaults... ");
    RegisterMap map;

    assert(map.read(reg::CONFIG1) == 0x96);
    assert(map.read(reg::CONFIG2) == 0xC0);
    assert(map.read(reg::CONFIG3) == 0x60);
    assert(map.read(reg::LOFF) == 0x00);
    assert(map.read(reg::CH1SET) == 0x61);
    assert(map.read(reg::CH8SET) == 0x61);
    assert(map.read(reg::GPIO) == 0x0F);
    assert(map.read(reg::MISC1) == 0x00);
    assert(map.read(reg::CONFIG4) == 0x00);

    for (uint8_t i = 0; i < reg::NUM_REGISTERS; i++) {
        assert(!map.isDirty(i));
    }

    printf("PASS\n");
}

void test_register_map_setfield() {
    printf("test_register_map_setfield... ");
    RegisterMap map;

    map.setField(reg::CONFIG1, reg::CONFIG1_DR_MASK, reg::CONFIG1_DR_SHIFT, 0);
    assert(map.getField(reg::CONFIG1, reg::CONFIG1_DR_MASK, reg::CONFIG1_DR_SHIFT) == 0);
    assert((map.read(reg::CONFIG1) & 0x98) == 0x90);
    assert(map.isDirty(reg::CONFIG1));

    map.setField(reg::CONFIG2, reg::CONFIG2_INT_CAL_MASK, reg::CONFIG2_INT_CAL_SHIFT, 1);
    assert(map.getField(reg::CONFIG2, reg::CONFIG2_INT_CAL_MASK, reg::CONFIG2_INT_CAL_SHIFT) == 1);
    assert((map.read(reg::CONFIG2) & 0xE8) == 0xC0);

    printf("PASS\n");
}

void test_fixed_bits_enforcement() {
    printf("test_fixed_bits_enforcement... ");
    RegisterMap map;

    map.write(reg::CONFIG1, 0x00);
    assert((map.read(reg::CONFIG1) & 0x98) == 0x90);

    map.write(reg::CONFIG3, 0x00);
    assert((map.read(reg::CONFIG3) & 0x60) == 0x60);

    map.write(reg::CONFIG4, 0xFF);
    assert((map.read(reg::CONFIG4) & 0xF5) == 0x00);

    printf("PASS\n");
}

void test_channel_proxy() {
    printf("test_channel_proxy... ");
    RegisterMap map;
    ChannelProxy ch0(map, 0);

    ch0.enabled(true);
    assert((map.read(reg::CH1SET) & 0x80) == 0x00);

    ch0.enabled(false);
    assert((map.read(reg::CH1SET) & 0x80) == 0x80);

    ch0.gain(ADS1299_PGA_GAIN_24);
    assert(map.getField(reg::CH1SET, reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT) == 6);

    ch0.srb2Closed(true);
    assert((map.read(reg::CH1SET) & 0x08) == 0x08);

    ch0.input(ADS1299_CI_TEST);
    assert(map.getField(reg::CH1SET, reg::CHNSET_MUX_MASK, reg::CHNSET_MUX_SHIFT) == 5);

    ch0.biasPEnable(true);
    assert((map.read(reg::BIAS_SENSP) & 0x01) == 0x01);

    ch0.biasNEnable(true);
    assert((map.read(reg::BIAS_SENSN) & 0x01) == 0x01);

    ch0.leadOffPEnable(true);
    assert((map.read(reg::LOFF_SENSP) & 0x01) == 0x01);

    ch0.leadOffFlip(true);
    assert((map.read(reg::LOFF_FLIP) & 0x01) == 0x01);

    ChannelProxy ch7(map, 7);
    ch7.biasPEnable(true);
    assert((map.read(reg::BIAS_SENSP) & 0x80) == 0x80);
    assert((map.read(reg::BIAS_SENSP) & 0x01) == 0x01);

    printf("PASS\n");
}

void test_channel_proxy_fluent() {
    printf("test_channel_proxy_fluent... ");
    RegisterMap map;
    ChannelProxy ch(map, 0);

    ch.enabled(true)
      .gain(ADS1299_PGA_GAIN_24)
      .srb2Closed(true)
      .input(ADS1299_CI_NORMAL)
      .biasPEnable(true)
      .biasNEnable(true)
      .leadOffPEnable(true)
      .leadOffNEnable(true)
      .leadOffFlip(false);

    assert(map.read(reg::CH1SET) == 0x68);
    assert(map.read(reg::BIAS_SENSP) == 0x01);
    assert(map.read(reg::BIAS_SENSN) == 0x01);
    assert(map.read(reg::LOFF_SENSP) == 0x01);
    assert(map.read(reg::LOFF_SENSN) == 0x01);
    assert(map.read(reg::LOFF_FLIP) == 0x00);

    printf("PASS\n");
}

void test_frontend_proxy() {
    printf("test_frontend_proxy... ");
    MockTransport transport;
    Config config(transport, 1);

    auto fe = config.frontend(0);

    fe.testSourceInternal(true)
      .testSignal2x(true)
      .testSignalFrequency(ADS1299_TF_2Hz);

    uint8_t cfg2 = config.registerMap(0).read(reg::CONFIG2);
    assert(cfg2 == 0xD5);

    fe.referenceBufferEnabled(true)
      .biasMeasurementEnabled(true)
      .biasReferenceInternal(true)
      .biasBufferEnabled(true)
      .biasSenseLeadOffEnabled(true);

    uint8_t cfg3 = config.registerMap(0).read(reg::CONFIG3);
    assert(cfg3 == 0xFE);

    fe.leadOffComparatorThreshold(ADS1299_CT_900ppt)
      .leadOffCurrent(ADS1299_LOC_6nA)
      .leadOffFrequency(ADS1299_LOF_fDR_4);

    uint8_t loff = config.registerMap(0).read(reg::LOFF);
    assert(loff == 0x43);

    fe.srb1Closed(true);
    assert((config.registerMap(0).read(reg::MISC1) & 0x20) == 0x20);

    fe.continuous(true);
    assert((config.registerMap(0).read(reg::CONFIG4) & 0x08) == 0x00);

    fe.leadOffComparatorsEnabled(true);
    assert((config.registerMap(0).read(reg::CONFIG4) & 0x02) == 0x02);

    printf("PASS\n");
}

void test_frontends_proxy_datarate() {
    printf("test_frontends_proxy_datarate... ");
    MockTransport transport;
    Config config(transport, 2);

    config.frontends().datarate(ADS1299_DR_16000);

    // DR=0 (16kSPS) on both
    assert(config.registerMap(0).getField(reg::CONFIG1,
        reg::CONFIG1_DR_MASK, reg::CONFIG1_DR_SHIFT) == 0);
    assert(config.registerMap(1).getField(reg::CONFIG1,
        reg::CONFIG1_DR_MASK, reg::CONFIG1_DR_SHIFT) == 0);

    printf("PASS\n");
}

void test_frontends_proxy_broadcast() {
    printf("test_frontends_proxy_broadcast... ");
    MockTransport transport;
    Config config(transport, 2);

    config.frontends()
        .referenceBufferEnabled(true)
        .biasBufferEnabled(true)
        .continuous(true);

    for (uint8_t i = 0; i < 2; i++) {
        assert(config.registerMap(i).getField(reg::CONFIG3,
            reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT) == 1);
        assert(config.registerMap(i).getField(reg::CONFIG3,
            reg::CONFIG3_PD_BIAS_MASK, reg::CONFIG3_PD_BIAS_SHIFT) == 1);
        assert(config.registerMap(i).getField(reg::CONFIG4,
            reg::CONFIG4_SINGLE_SHOT_MASK, reg::CONFIG4_SINGLE_SHOT_SHIFT) == 0);
    }

    printf("PASS\n");
}

void test_channels_proxy_broadcast() {
    printf("test_channels_proxy_broadcast... ");
    MockTransport transport;
    Config config(transport, 1);

    config.registerMap(0).write(reg::ID, 0x3E);  // 8ch
    config.registerMap(0).clearAllDirty();

    config.channels()
        .enabled(true)
        .gain(ADS1299_PGA_GAIN_24)
        .srb2Closed(true)
        .input(ADS1299_CI_NORMAL)
        .biasPEnable(true)
        .biasNEnable(true);

    for (uint8_t i = 0; i < 8; i++) {
        assert(config.registerMap(0).read(reg::CH1SET + i) == 0x68);
    }

    assert(config.registerMap(0).read(reg::BIAS_SENSP) == 0xFF);
    assert(config.registerMap(0).read(reg::BIAS_SENSN) == 0xFF);

    config.channels().channel(0).input(ADS1299_CI_TEST);
    assert(config.registerMap(0).getField(reg::CH1SET,
        reg::CHNSET_MUX_MASK, reg::CHNSET_MUX_SHIFT) == 5);

    printf("PASS\n");
}

void test_gpio_proxy() {
    printf("test_gpio_proxy... ");
    MockTransport transport;
    Config config(transport, 1);

    auto fe = config.frontend(0);

    assert(config.registerMap(0).read(reg::GPIO) == 0x0F);

    fe.gpio(0).isInput(false);
    assert((config.registerMap(0).read(reg::GPIO) & 0x01) == 0x00);

    fe.gpio(0).value(true);
    assert((config.registerMap(0).read(reg::GPIO) & 0x10) == 0x10);

    assert(fe.gpio(0).value() == true);
    assert((config.registerMap(0).read(reg::GPIO) & 0x08) == 0x08);

    printf("PASS\n");
}

void test_apply() {
    printf("test_apply... ");
    MockTransport transport;
    Config config(transport, 2);

    config.frontend(0).referenceBufferEnabled(true);
    config.frontend(1).biasBufferEnabled(true);

    assert(config.apply());
    assert(transport.cmdCount == 2);  // SDATAC x2
    assert(transport.writeCount == 2);
    assert(transport.regs[0][reg::CONFIG3] == config.registerMap(0).read(reg::CONFIG3));
    assert(transport.regs[1][reg::CONFIG3] == config.registerMap(1).read(reg::CONFIG3));

    printf("PASS\n");
}

void test_apply_only_dirty() {
    printf("test_apply_only_dirty... ");
    MockTransport transport;
    Config config(transport, 1);

    transport.writeCount = 0;
    transport.cmdCount = 0;
    assert(config.apply());
    assert(transport.cmdCount == 1);
    assert(transport.writeCount == 0);

    printf("PASS\n");
}

void test_apply_failure() {
    printf("test_apply_failure... ");
    MockTransport transport;
    Config config(transport, 1);

    transport.failNext = true;
    config.frontend(0).referenceBufferEnabled(true);
    assert(!config.apply());
    assert(config.lastError() != nullptr);

    printf("PASS\n");
}

void test_show_registers() {
    printf("test_show_registers... ");
    MockTransport transport;
    Config config(transport, 1);

    StringPrint out;
    config.showRegisters(out);

    // Should contain ADC header
    assert(strstr(out.buf, "ADC 0") != nullptr);
    // CONFIG1 shown as plain hex (no special markers)
    assert(strstr(out.buf, "CONFIG1") != nullptr);

    printf("PASS\n");
}

void test_global_channel_indexing() {
    printf("test_global_channel_indexing... ");
    MockTransport transport;
    Config config(transport, 2);

    config.registerMap(0).write(reg::ID, 0x3E);  // 8ch
    config.registerMap(1).write(reg::ID, 0x3E);
    config.registerMap(0).clearAllDirty();
    config.registerMap(1).clearAllDirty();

    assert(config.totalChannels() == 16);

    config.channel(0).gain(ADS1299_PGA_GAIN_1);
    assert(config.registerMap(0).getField(reg::CH1SET,
        reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT) == 0);

    config.channel(8).gain(ADS1299_PGA_GAIN_2);
    assert(config.registerMap(1).getField(reg::CH1SET,
        reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT) == 1);

    config.channel(15).gain(ADS1299_PGA_GAIN_4);
    assert(config.registerMap(1).getField(reg::CH8SET,
        reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT) == 2);

    printf("PASS\n");
}

void test_last_write_wins() {
    printf("test_last_write_wins... ");
    MockTransport transport;
    Config config(transport, 1);

    config.frontends()
        .testSignalFrequency(ADS1299_TF_2Hz)
        .testSignalFrequency(ADS1299_TF_1Hz)
        .testSignalFrequency(ADS1299_TF_DC);

    assert(config.registerMap(0).getField(reg::CONFIG2,
        reg::CONFIG2_CAL_FREQ_MASK, reg::CONFIG2_CAL_FREQ_SHIFT) == ADS1299_TF_DC);

    printf("PASS\n");
}

void test_legacy_wreg_rreg() {
    printf("test_legacy_wreg_rreg... ");
    MockTransport transport;
    Config config(transport, 1);

    config.frontend(0).wreg(reg::CONFIG2, 0xD5);
    assert(config.frontend(0).rreg(reg::CONFIG2) == 0xD5);

    printf("PASS\n");
}

void test_num_frontends() {
    printf("test_num_frontends... ");
    MockTransport transport;

    Config config1(transport, 1);
    assert(config1.numFrontends() == 1);

    Config config4(transport, 4);
    assert(config4.numFrontends() == 4);

    Config config128(transport, 128);
    assert(config128.numFrontends() == 128);

    // Clamp to MAX_FRONTENDS
    Config configOver(transport, 200);
    assert(configOver.numFrontends() == 128);

    printf("PASS\n");
}

void test_leadoff_status_buffer() {
    printf("test_leadoff_status_buffer... ");
    MockTransport transport;
    Config config(transport, 3);

    config.registerMap(0).write(reg::LOFF_STATP, 0xA5);
    config.registerMap(1).write(reg::LOFF_STATP, 0x3C);
    config.registerMap(2).write(reg::LOFF_STATP, 0x42);

    uint8_t buf[4] = {0};
    uint8_t count = config.frontends().leadOffP(buf, 4);
    assert(count == 3);
    assert(buf[0] == 0xA5);
    assert(buf[1] == 0x3C);
    assert(buf[2] == 0x42);
    assert(buf[3] == 0);  // Unused

    // Test with smaller buffer
    uint8_t small[2] = {0};
    count = config.frontends().leadOffP(small, 2);
    assert(count == 2);
    assert(small[0] == 0xA5);
    assert(small[1] == 0x3C);

    printf("PASS\n");
}

void test_bias_leadoff_not_connected() {
    printf("test_bias_leadoff_not_connected... ");
    MockTransport transport;
    Config config(transport, 1);

    // Default: BIAS_STAT=0 (connected)
    assert(config.frontend(0).biasLeadOffNotConnected() == false);

    // Set BIAS_STAT=1 (not connected) — simulating hardware read
    uint8_t cfg3 = config.registerMap(0).read(reg::CONFIG3);
    config.registerMap(0).write(reg::CONFIG3, cfg3 | 0x01);
    assert(config.frontend(0).biasLeadOffNotConnected() == true);

    printf("PASS\n");
}

void test_load() {
    printf("test_load... ");
    MockTransport transport;
    Config config(transport, 2);

    // Set up hardware registers
    transport.regs[0][reg::CONFIG1] = 0x96;
    transport.regs[0][reg::CONFIG2] = 0xD5;
    transport.regs[0][reg::ID] = 0x3E;
    transport.regs[1][reg::CONFIG1] = 0x96;
    transport.regs[1][reg::CONFIG2] = 0xC0;
    transport.regs[1][reg::ID] = 0x3E;

    assert(config.load());
    assert(config.lastError() == nullptr);

    // Verify shadow map matches hardware
    assert(config.registerMap(0).read(reg::CONFIG2) == 0xD5);
    assert(config.registerMap(1).read(reg::CONFIG2) == 0xC0);

    // Verify not dirty after load
    assert(!config.registerMap(0).isDirty(reg::CONFIG2));
    assert(!config.registerMap(1).isDirty(reg::CONFIG2));

    printf("PASS\n");
}

void test_load_failure() {
    printf("test_load_failure... ");
    MockTransport transport;
    Config config(transport, 1);

    transport.failNext = true;
    assert(!config.load());
    assert(config.lastError() != nullptr);

    printf("PASS\n");
}

void test_apply_diff() {
    printf("test_apply_diff... ");
    MockTransport transport;

    // "current" represents hardware state
    Config current(transport, 2);
    current.registerMap(0).write(reg::ID, 0x3E);
    current.registerMap(1).write(reg::ID, 0x3E);
    // Set DAISY_EN=0 (daisy mode) and CLK_EN=1 on frontend 0
    current.registerMap(0).write(reg::CONFIG1, 0xB6);  // CLK_EN=1, DR=250
    current.registerMap(1).write(reg::CONFIG1, 0x96);   // CLK_EN=0, DR=250
    current.registerMap(0).clearAllDirty();
    current.registerMap(1).clearAllDirty();

    // "desired" config — change datarate but not DAISY_EN/CLK_EN
    Config desired(transport, 2);
    desired.registerMap(0).write(reg::ID, 0x3E);
    desired.registerMap(1).write(reg::ID, 0x3E);
    desired.frontends().datarate(ADS1299_DR_16000);
    desired.frontend(0).referenceBufferEnabled(true);

    transport.reset();
    assert(desired.apply(current));

    // Should have written CONFIG1 on both (DR changed) and CONFIG3 on FE0
    // CONFIG1 should preserve CLK_EN and DAISY_EN from current
    // FE0 CONFIG1: desired DR=0, preserved CLK_EN=1, DAISY_EN=0 → 0xB0
    assert(transport.regs[0][reg::CONFIG1] == 0xB0);
    // FE1 CONFIG1: desired DR=0, preserved CLK_EN=0, DAISY_EN=0 → 0x90
    assert(transport.regs[1][reg::CONFIG1] == 0x90);

    // CONFIG3 only changed on FE0 (referenceBufferEnabled)
    assert(transport.regs[0][reg::CONFIG3] != 0);

    printf("PASS\n");
}

void test_apply_diff_no_changes() {
    printf("test_apply_diff_no_changes... ");
    MockTransport transport;

    Config current(transport, 1);
    Config desired(transport, 1);

    // Both have same defaults
    transport.reset();
    assert(desired.apply(current));
    // SDATAC sent but no writes (everything matches)
    assert(transport.cmdCount == 1);
    assert(transport.writeCount == 0);

    printf("PASS\n");
}

void test_apply_diff_preserves_daisy_clk() {
    printf("test_apply_diff_preserves_daisy_clk... ");
    MockTransport transport;

    Config current(transport, 1);
    // Current hardware has DAISY_EN=1 (multiple readback), CLK_EN=1
    current.registerMap(0).write(reg::CONFIG1, 0xF6);  // DAISY=1, CLK=1, DR=250
    current.registerMap(0).clearAllDirty();

    Config desired(transport, 1);
    // Desired: same DR=250 (0x96 default has DAISY=0, CLK=0)
    // The DAISY_EN and CLK_EN from desired should be IGNORED; current's preserved

    transport.reset();
    assert(desired.apply(current));

    // CONFIG1 should differ: desired has DAISY=0,CLK=0 but current has DAISY=1,CLK=1
    // After preserve: (0x96 & ~0x60) | (0xF6 & 0x60) = 0x96 & 0x9F | 0x60 = 0x96 | 0x60 = 0xF6
    // But that equals current, so no write should happen
    assert(transport.writeCount == 0);

    printf("PASS\n");
}

void test_serialize_deserialize() {
    printf("test_serialize_deserialize... ");
    MockTransport transport;
    Config config(transport, 2);

    config.frontends()
        .datarate(ADS1299_DR_16000)
        .referenceBufferEnabled(true);
    config.channels().gain(ADS1299_PGA_GAIN_24);

    // Serialize
    size_t size = config.serializedSize();
    assert(size == 2 + 2 * 24);  // header + 2 frontends * 24 regs

    uint8_t buf[256];
    size_t written = config.serialize(buf, sizeof(buf));
    assert(written == size);
    assert(buf[0] == SERIAL_VERSION);
    assert(buf[1] == 2);  // numFrontends

    // Deserialize into a new config
    Config config2(transport, 2);
    assert(config2.deserialize(buf, written));

    // Verify register values match
    for (uint8_t fe = 0; fe < 2; fe++) {
        for (uint8_t r = 0; r < reg::NUM_REGISTERS; r++) {
            assert(config2.registerMap(fe).read(r) == config.registerMap(fe).read(r));
        }
    }

    printf("PASS\n");
}

void test_deserialize_config_down() {
    printf("test_deserialize_config_down... ");
    MockTransport transport;

    // Source: 4 ADCs
    Config src(transport, 4);
    src.frontends().referenceBufferEnabled(true);
    src.registerMap(0).write(reg::ID, 0x3E);  // 8ch

    uint8_t buf[512];
    size_t written = src.serialize(buf, sizeof(buf));
    assert(written > 0);

    // Target: 1 ADC — config-down should succeed
    Config target(transport, 1);
    assert(target.deserialize(buf, written));

    // Frontend 0 should have the source's settings
    assert(target.registerMap(0).getField(reg::CONFIG3,
        reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT) == 1);

    // Registers should be dirty (ready for apply)
    assert(target.registerMap(0).isDirty(reg::CONFIG3));

    printf("PASS\n");
}

void test_deserialize_config_up() {
    printf("test_deserialize_config_up... ");
    MockTransport transport;

    // Source: 1 ADC
    Config src(transport, 1);
    uint8_t buf[256];
    size_t written = src.serialize(buf, sizeof(buf));

    // Target: 4 ADCs — config-up should fail
    Config target(transport, 4);
    assert(!target.deserialize(buf, written));
    assert(target.lastError() != nullptr);
    assert(strstr(target.lastError(), "Config-up") != nullptr);

    printf("PASS\n");
}

void test_deserialize_bad_version() {
    printf("test_deserialize_bad_version... ");
    MockTransport transport;
    Config config(transport, 1);

    uint8_t buf[] = {0xFF, 1};  // Bad version
    assert(!config.deserialize(buf, sizeof(buf)));

    printf("PASS\n");
}

void test_deserialize_buffer_too_small() {
    printf("test_deserialize_buffer_too_small... ");
    MockTransport transport;
    Config config(transport, 1);

    uint8_t buf[] = {SERIAL_VERSION, 1};  // Header only, no register data
    assert(!config.deserialize(buf, sizeof(buf)));

    printf("PASS\n");
}

void test_serialize_buffer_too_small() {
    printf("test_serialize_buffer_too_small... ");
    MockTransport transport;
    Config config(transport, 1);

    uint8_t buf[4];  // Too small
    assert(config.serialize(buf, sizeof(buf)) == 0);

    printf("PASS\n");
}

void test_128_frontends() {
    printf("test_128_frontends... ");
    MockTransport transport;
    Config config(transport, 128);

    assert(config.numFrontends() == 128);

    // Set all channels to a specific gain
    // (totalChannels with default ID reg = 8ch per frontend = 1024)
    // Just test a few frontends to keep test fast
    config.frontend(0).referenceBufferEnabled(true);
    config.frontend(127).referenceBufferEnabled(true);

    assert(config.registerMap(0).getField(reg::CONFIG3,
        reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT) == 1);
    assert(config.registerMap(127).getField(reg::CONFIG3,
        reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT) == 1);

    // Middle frontend should still be default
    assert(config.registerMap(64).getField(reg::CONFIG3,
        reg::CONFIG3_PD_REFBUF_MASK, reg::CONFIG3_PD_REFBUF_SHIFT) == 0);

    printf("PASS\n");
}

void test_128_frontends_channels() {
    printf("test_128_frontends_channels... ");
    MockTransport transport;
    Config config(transport, 4);

    // Set all to 8ch
    for (uint8_t i = 0; i < 4; i++) {
        config.registerMap(i).write(reg::ID, 0x3E);
        config.registerMap(i).clearAllDirty();
    }

    assert(config.totalChannels() == 32);

    // Channel 31 = frontend 3, local channel 7
    config.channel(31).gain(ADS1299_PGA_GAIN_12);
    assert(config.registerMap(3).getField(reg::CH8SET,
        reg::CHNSET_GAIN_MASK, reg::CHNSET_GAIN_SHIFT) == 5);

    printf("PASS\n");
}

void test_channel_out_of_bounds() {
    printf("test_channel_out_of_bounds... ");
    MockTransport transport;
    Config config(transport, 2);

    // Default ID register NU_CH=10 → 8ch per frontend, totalChannels = 16
    assert(config.totalChannels() == 16);

    // Accessing channel 9999 should not crash (clamps to last frontend, ch 0)
    auto ch = config.channel(9999);

    // Should be able to set properties without crashing
    ch.enabled(true);

    printf("PASS\n");
}

int main() {
    printf("=== ADS1299Config Tests ===\n\n");

    test_register_map_defaults();
    test_register_map_setfield();
    test_fixed_bits_enforcement();
    test_channel_proxy();
    test_channel_proxy_fluent();
    test_frontend_proxy();
    test_frontends_proxy_datarate();
    test_frontends_proxy_broadcast();
    test_channels_proxy_broadcast();
    test_gpio_proxy();
    test_apply();
    test_apply_only_dirty();
    test_apply_failure();
    test_show_registers();
    test_global_channel_indexing();
    test_last_write_wins();
    test_legacy_wreg_rreg();
    test_num_frontends();
    test_leadoff_status_buffer();
    test_bias_leadoff_not_connected();
    test_load();
    test_load_failure();
    test_apply_diff();
    test_apply_diff_no_changes();
    test_apply_diff_preserves_daisy_clk();
    test_serialize_deserialize();
    test_deserialize_config_down();
    test_deserialize_config_up();
    test_deserialize_bad_version();
    test_deserialize_buffer_too_small();
    test_serialize_buffer_too_small();
    test_128_frontends();
    test_128_frontends_channels();
    test_channel_out_of_bounds();

    printf("\n=== All tests passed! ===\n");
    return 0;
}
