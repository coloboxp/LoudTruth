# Connections

```
ESP32-S3 WROOM                   ST7565 LCD Display 128x64
+---------------+                 +-----------------+
|           3.3V|---------------->|VCC              |
|            GND|---------------->|GND              |
|     GPIO23(MO)|---------------->|MOSI             |
|     GPIO18(SK)|---------------->|SCK              |
|      GPIO5(CS)|---------------->|CS               |
|     GPIO16(DC)|---------------->|DC               |
|      GPIO4(RS)|---------------->|RST              |
|      GPIO2(BL)|---------------->|BL               |
+---------------+                 +-----------------+

                                 SD Card Module
                                 +-----------------+
                                 |VCC              |<----+
                                 |GND              |<-+  |
                                 |MOSI             |<-|--|--(shared with LCD)
                                 |SCK              |<-|--|--(shared with LCD)
                                 |CS               |<-|--|-GPIO17
                                 |MISO             |<-|--|-GPIO19
                                 +-----------------+  |  |
                                                      |  |
                                 Sound Sensor         |  |
                                 +-----------------+  |  |
                                 |VCC              |  |  |
                                 |GND              |  |  |
                                 |OUT              |---|-GPIO36
                                 +-----------------+  |  |
                                                      |  |
                                 NeoPixel Strip       |  |
                                 +-----------------+  |  |
                                 |VCC(5V)          |  <--|-5V from USB
                                 |GND              |  <--+
                                 |DIN              |  <----GPIO21
                                 +-----------------+

                                 Piezo Speaker
                                 +-----------------+
                                 |PIN1             |<----GPIO26
                                 |PIN2             |<----GND
                                 +-----------------+

Power Distribution
+-----------------+
|ESP32 3.3V       |---> LCD VCC, SD Card VCC, Sound Sensor VCC, NeoPixel Strip VCC
|ESP32 GND        |---> All GND connections
+-----------------+
```

Notes:

1. SPI Bus Configuration:

   - MOSI: GPIO23 (shared between LCD and SD Card)
   - SCK: GPIO18 (shared between LCD and SD Card)
   - MISO: GPIO19 (SD Card only)

2. Display Connections:

   - CS: GPIO5 (Chip Select)
   - DC: GPIO16 (Data/Command)
   - RST: GPIO4 (Reset)
   - BL: GPIO2 (Backlight)

3. SD Card Specific:

   - CS: GPIO17 (Independent CS pin)
   - Uses shared SPI bus

4. Analog Input:

   - Sound Sensor: GPIO36 (ADC1_CH0)
   - 12-bit resolution (0-4095)

5. LED Control:

   - NeoPixel Strip: GPIO21
   - 8 LEDs in series

6. Power Management:

   - All 3.3V devices powered from ESP32's regulator
   - NeoPixel strip requires 5V from USB
   - All GND pins must be connected together
   - Ensure clean power supply for accurate ADC readings
