# Noise Level Monitor

An ESP32-based device that monitors ambient noise levels, categorizes them, and logs the data for analysis.

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
