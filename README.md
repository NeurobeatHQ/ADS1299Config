# ADS1299Config

A transport-agnostic Arduino/PlatformIO library for configuring boards with [ADS1299](https://www.ti.com/product/ADS1299) analog front-end chips. It maintains a shadow register map in RAM and pushes changes to hardware through a user-provided SPI driver. The library handles register bit fields, fixed-bit enforcement, and dirty tracking so your application code reads like a datasheet rather than a collection of bitmasks. It supports up to 128 ADC chips (1024 channels) addressed via separate CS pins. Daisy chaining is used for continuous data readback and is managed externally — this library is purely for register configuration.

The API is organized as a hierarchy of lightweight proxy objects. `config.frontends()` broadcasts a setting to every chip, `config.frontend(i)` targets one chip, `config.channels()` broadcasts to every channel across all chips, and `config.channel(i)` targets a single channel by global index. All setters return `*this` for chaining and use affirmative naming that matches the datasheet (`referenceBufferEnabled(true)`, `continuous(true)`, `srb2Closed(true)`). Inverted register bits like `PDn` and `SINGLE_SHOT` are handled internally so `enabled(true)` and `continuous(true)` do what you'd expect.

```cpp
#include <ADS1299Config.h>
using namespace ads1299_config;

Config config(spiTransport, 2);  // 2 chips, 16 channels

config.frontends()
    .datarate(ADS1299_DR_250)
    .referenceBufferEnabled(true)
    .biasBufferEnabled(true)
    .continuous(true);

config.channels()
    .enabled(true)
    .gain(ADS1299_PGA_GAIN_24)
    .input(ADS1299_CI_NORMAL);

config.channel(0).input(ADS1299_CI_TEST);
config.apply();
```

To use the library you implement the `Transport` interface with three methods — `writeRegister`, `readRegister`, and `sendCommand` — each taking a `frontendIndex` that identifies which chip to talk to. `apply()` writes only dirty registers, `apply(currentConfig)` does a diff-based write that also preserves hardware-managed `DAISY_EN` and `CLK_EN` bits, and `load()` reads all registers from hardware into the shadow map. Configs can be serialized to a byte buffer and deserialized into a system with the same or fewer chips, making it easy to store settings in EEPROM or send them over a network. Install via the Arduino Library Manager or add the library to your PlatformIO `lib_deps`.
