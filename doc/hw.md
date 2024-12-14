# Hardware Architecture

```mermaid
graph TD
    ESP32[ESP32-S3 WROOM] --> |SPI| LCD[LCD Display<br>ST7565 ERC12864]
    ESP32 --> |SPI| SD[SD Card Module]
    ESP32 --> |ADC| SOUND[Sound Sensor]
    ESP32 --> |Digital| LED[NeoPixel LED Strip]

    subgraph SPI Bus
        LCD -.->|MOSI: GPIO23<br>SCK: GPIO18|
    end

    subgraph Display Pins
        LCD -->|CS: GPIO5<br>DC: GPIO16<br>RST: GPIO4<br>BL: GPIO2| ESP32
    end

    subgraph SD Card Pins
        SD -->|CS: GPIO17<br>MISO: GPIO19| ESP32
    end

    subgraph Analog Input
        SOUND -->|OUT: GPIO15| ESP32
    end

    subgraph LED Strip
        LED -->|DIN: GPIO21| ESP32
    end
```

## Pin Assignments

### Display (ST7565 ERC12864)

- CS: GPIO5 (PIN_DISPLAY_CHIP_SELECT)
- DC: GPIO16 (PIN_DISPLAY_DATA_COMMAND)
- RESET: GPIO4 (PIN_DISPLAY_RESET)
- BACKLIGHT: GPIO2 (PIN_DISPLAY_BACKLIGHT)
- MOSI: GPIO23 (Shared SPI)
- SCK: GPIO18 (Shared SPI)

### SD Card Module

- CS: GPIO17 (PIN_SD_CS)
- MOSI: GPIO23 (Shared SPI)
- MISO: GPIO19
- SCK: GPIO18 (Shared SPI)
- VCC: 3.3V
- GND: GND

### Sound Sensor

- OUT: GPIO15 (PIN_SOUND_SENSOR)
- VCC: 3.3V
- GND: GND

### NeoPixel LED Strip

- DIN: GPIO21 (PIN_LED_STRIP)
- VCC: 5V
- GND: GND

## Hardware Specifications

### Sound Sensor

- Operating Voltage: 3.3V
- Voltage Gain: 26 dB
- Microphone sensitivity: -60~-56dBV/Pa
- ADC Input Range: 0-4095 (12-bit)

### Display

- Type: ST7565 ERC12864
- Resolution: 128x64 pixels
- Interface: 4-Wire SPI
- Backlight: LED (active LOW)
- Contrast: Software adjustable

### LED Strip

- Type: WS2812B (NeoPixel)
- Number of LEDs: 8
- Data Rate: 800kHz
- Color Order: GRB
- Power: 5V, ~60mA per LED at full brightness

### SD Card

- Interface: SPI
- Format: FAT32
- Speed: Standard mode (25MHz)

## Power Requirements

- Main Board: 5V via USB
- Logic Level: 3.3V
- Peak Current: ~500mA
- Average Current: ~200mA

## Notes

1. SPI bus (MOSI, SCK) is shared between display and SD card
2. All 3.3V peripherals are powered from the ESP32's onboard regulator
3. LED strip requires separate 5V power from USB
4. Sound sensor requires clean 3.3V power for accurate readings
5. Display contrast is software-adjusted for optimal visibility
