# Noise Level Monitor

An ESP32-based device that monitors ambient noise levels, categorizes them, and logs the data for analysis.

## Quick Start

1. Install PlatformIO
2. Connect the hardware according to the [hardware documentation](doc/hw.md)
3. Build and flash:

   ```bash
   pio run -t upload
   ```

## Features

- Real-time noise level monitoring with 12-bit ADC resolution
- Dynamic noise categorization using Exponential Moving Average (EMA)
- 32-sample averaging for improved noise reduction
- LCD display with current levels and statistics
- WiFi connectivity status indicator
- ThingSpeak integration for remote monitoring
- Independent statistics tracking for 1-min, 15-min, and daily periods
- LED strip with fast response time (50ms updates)
- SD card logging for data analysis
- Configurable thresholds and parameters

## Recent Updates

### Display and Performance Improvements

- Separated LED indicator refresh (50ms) from display updates (500ms)
- Added WiFi and ThingSpeak connection status indicators
- Improved statistics tracking with independent time windows
- Optimized initialization sequence to prevent watchdog resets

### Technical Improvements

- Updated ADC value handling to use hardware maximum
- Improved statistics tracking accuracy
- Enhanced system stability with proper component initialization
- Added independent time windows for statistics collection

## Documentation

Detailed documentation is available in the [doc](doc/) directory:

- [Full Project Documentation](doc/README.md)
- [Hardware Setup](doc/hw.md)
- [Architecture Overview](doc/arch.md)
- [Component Architecture](doc/componentarch.md)
- [Data Flow](doc/dataflow.md)
- [Flow and Timing](doc/flowNtiming.md)
- [State Management](doc/state.md)
- [Component Dependencies](doc/componentdeps.md)

## Hardware Support

This project is specifically configured for the Freenove ESP32-S3 WROOM board with:

- 8MB Flash
- 8MB PSRAM
- USB-CDC enabled
- 12-bit ADC resolution
- Optimized ADC settings for sound measurement

## License

This project is licensed under the MIT License - see the LICENSE file for details.
