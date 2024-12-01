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
- Categories: OK, Regular, High, and Toxic based on baseline noise level
- LCD display showing:
  - Current noise level and category
  - 1-minute, 15-minute statistics (avg, min, max)
  - Real-time plot of the last minute
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
  CS    → GPIO19
  DC    → GPIO21
  MOSI  → GPIO11
  SCK   → GPIO10
  RESET → GPIO20
  BL    → GPIO9

SD Card Module (SPI):
  CS    → GPIO34
  MOSI  → GPIO11 (shared with display)
  MISO  → GPIO13
  SCK   → GPIO10 (shared with display)

Sound Sensor:
  OUT   → GPIO A0 (ADC)
  VCC   → 3.3V
  GND   → GND
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
