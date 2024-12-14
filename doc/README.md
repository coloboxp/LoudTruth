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
- Independent statistics tracking:
  - 1-minute window
  - 15-minute window
  - 24-hour window
- Visual feedback through:
  - LCD display showing current levels and statistics (500ms refresh)
  - 8-LED NeoPixel strip for instant level indication (50ms refresh)
  - Color-coded LED feedback (Green to Red)
  - WiFi and ThingSpeak connection status indicators
- SD card logging for historical analysis
- ThingSpeak integration for remote monitoring

## Recent Technical Changes

### Display System

- Decoupled display refresh (500ms) from LED updates (50ms)
- Added connection status indicators for WiFi and ThingSpeak
- Improved display initialization sequence

### Statistics Processing

- Independent time windows for 1-min, 15-min, and daily statistics
- Improved average calculation to prevent overflow
- Updated min/max value handling using hardware ADC maximum

### System Stability

- Enhanced initialization sequence with proper delays
- Improved watchdog timer handling
- Optimized memory usage in statistics tracking

## Hardware Requirements

- Freenove ESP32-S3 WROOM
- Sound sensor module (12-bit ADC input)
- 128x64 LCD Display (ST7565 ERC12864)
- Micro SD Card module
- Micro SD Card (FAT32 formatted)
- 8-LED NeoPixel strip
- Piezo speaker for alerts

## Pin Connections

See [connections.md](connections.md) for detailed wiring diagram.

Key Connections:

- Sound Sensor: GPIO36 (ADC1_CH0)
- NeoPixel Strip: GPIO21
- Piezo Speaker: GPIO26
- Display and SD card share SPI bus (MOSI: GPIO23, SCK: GPIO18)

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
