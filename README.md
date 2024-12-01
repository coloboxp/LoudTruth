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

- Real-time noise level monitoring and categorization
- LCD display with current levels and statistics
- SD card logging for data analysis
- Configurable thresholds and parameters

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

## License

This project is licensed under the MIT License - see the LICENSE file for details.
