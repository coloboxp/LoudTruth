# Noise Level Monitor - Technical Documentation

## ADC Configuration

The project uses the ESP32-S3's ADC with optimized settings:

- 12-bit resolution (0-4095 range)
- 11dB attenuation for 3.3V reference voltage
- 32-sample averaging for noise reduction
- Optimized clock divider settings

### Sound Sensor Specifications

- Operating Voltage: 3.3V
- Voltage Gain: 26 dB
- Microphone sensitivity: -60~-56dBV/Pa
- ADC Input Range: 0-4095 (12-bit)

## Board Configuration

Using Freenove ESP32-S3 WROOM with:

- 8MB Flash / 8MB PSRAM
- 240MHz CPU frequency
- USB-CDC enabled for programming and monitoring
- DIO flash mode with 80MHz flash frequency
- Built-in debugging support

## Features

- Real-time noise level monitoring using sound sensor
- Dynamic noise categorization using Exponential Moving Average (EMA)
- Visual feedback through:
  - LCD display showing current levels and statistics
  - 8-LED NeoPixel strip for instant level indication
  - Color-coded LED feedback (Green to Red)
- SD card logging for historical analysis

## Hardware Requirements

- Freenove ESP32-S3 WROOM
- Sound sensor module
- 128x64 LCD Display (ST7565 ERC12864)
- Micro SD Card module
- Micro SD Card (FAT32 formatted)

## Pin Connections

```
ESP32-S3 WROOM Board Layout:

                  ┌──────────────────┐
                  │       USB        │
                  └──────────────────┘
                  ┌──────────────────┐
              3V3 │ ○             ○ │ GND
              RST │ ○             ○ │ U0T
              4   │ ○             ○ │ U0R
              5   │ ○             ○ │ 6
              7   │ ○             ○ │ 8
              9   │ ○  ESP32-S3   ○ │ 10
              11  │ ○   WROOM     ○ │ 12
              13  │ ○             ○ │ 14
              15  │ ○             ○ │ 16
              17  │ ○             ○ │ 18
              19  │ ○             ○ │ 20
              21  │ ○             ○ │ 34
              35  │ ○             ○ │ 36
              37  │ ○             ○ │ 38
              39  │ ○             ○ │ 40
              41  │ ○             ○ │ 42
              43  │ ○             ○ │ 44
              45  │ ○             ○ │ 46
              47  │ ○             ○ │ 48
              GND │ ○             ○ │ 5V
                  └──────────────────┘

Connections:

LCD Display (ST7565 ERC12864 - 4-Wire SPI):
  CS    → GPIO5  (PIN_DISPLAY_CHIP_SELECT)
  DC    → GPIO16 (PIN_DISPLAY_DATA_COMMAND)
  MOSI  → GPIO23 (PIN_MOSI, shared with SD card)
  SCK   → GPIO18 (PIN_SCK, shared with SD card)
  RESET → GPIO4  (PIN_DISPLAY_RESET)
  BL    → GPIO2  (PIN_DISPLAY_BACKLIGHT)

SD Card Module (SPI):
  CS    → GPIO17 (PIN_SD_CS)
  MOSI  → GPIO23 (PIN_MOSI, shared with display)
  MISO  → GPIO19 (PIN_MISO)
  SCK   → GPIO18 (PIN_SCK, shared with display)
  VCC   → 3.3V
  GND   → GND

Sound Sensor:
  OUT   → GPIO15 (PIN_SOUND_SENSOR)
  VCC   → 3.3V
  GND   → GND

NeoPixel LED Strip:
  DIN   → GPIO21 (PIN_LED_STRIP)
  VCC   → 5V
  GND   → GND

Notes:
- SPI bus (MOSI, SCK) is shared between the display and SD card
- All pins are on the right side of the board for better accessibility
- Power connections (3.3V, 5V, GND) can be connected to any appropriate pins on the board
```

## Data Logging

The device logs data to `noise_log.csv` on the SD card with the following format:

## Configuration

All configurable parameters are in `include/config.h`:

- EMA smoothing factors
- Noise threshold factors
- Sampling intervals
- Display settings

## Building and Flashing

This project uses PlatformIO. To build and flash:

```bash
pio run -t upload
```

## Project Structure

See other documentation files for detailed information:

- [Hardware Documentation](hw.md)
- [Flow and Timing](flowNtiming.md)
- [Architecture](arch.md)
- [Component Dependencies](componentdeps.md)
- [State Management](state.md)
- [Data Flow](dataflow.md)
- [Component Architecture](componentarch.md)
