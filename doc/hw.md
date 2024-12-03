```mermaid
graph TD
    ESP32[ESP32-S3 WROOM] --> |SPI| LCD[LCD Display<br>ST7565 ERC12864]
    ESP32 --> |SPI| SD[SD Card Module]
    ESP32 --> |ADC| SOUND[Sound Sensor]
    ESP32 --> |Digital| LED[NeoPixel LED Strip]

    subgraph SPI Bus
        LCD -.->|MOSI: GPIO11<br>SCK: GPIO10|
    end

    subgraph Display Pins
        LCD -->|CS: GPIO19<br>DC: GPIO21<br>RST: GPIO20<br>BL: GPIO9| ESP32
    end

    subgraph SD Card Pins
        SD -->|CS: GPIO34<br>MISO: GPIO13| ESP32
    end

    subgraph Analog Input
        SOUND -->|OUT: GPIO A0| ESP32
    end

    subgraph LED Strip
        LED -->|DIN: GPIO21| ESP32
    end
```
