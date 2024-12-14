#pragma once

#include <Arduino.h>

// Hardware Configuration
namespace pins
{
// SPI Bus (shared between display and SD)
#ifndef PIN_MOSI
    constexpr uint8_t MOSI = 11;
#else
    constexpr uint8_t MOSI = PIN_MOSI;
#endif

#ifndef PIN_MISO
    constexpr uint8_t MISO = 13;
#else
    constexpr uint8_t MISO = PIN_MISO;
#endif

#ifndef PIN_SCK
    constexpr uint8_t SCK = 10;
#else
    constexpr uint8_t SCK = PIN_SCK;
#endif

    // Display specific
    namespace display
    {
        // Add SPI pins to display namespace
        constexpr uint8_t MOSI = pins::MOSI; // Use the shared SPI pins
        constexpr uint8_t SCK = pins::SCK;   // Use the shared SPI pins

#ifndef PIN_DISPLAY_CHIP_SELECT
        constexpr uint8_t CS = 19;
#else
        constexpr uint8_t CS = PIN_DISPLAY_CHIP_SELECT;
#endif

#ifndef PIN_DISPLAY_DATA_COMMAND
        constexpr uint8_t DC = 21;
#else
        constexpr uint8_t DC = PIN_DISPLAY_DATA_COMMAND;
#endif

#ifndef PIN_DISPLAY_RESET
        constexpr uint8_t RESET = 20;
#else
        constexpr uint8_t RESET = PIN_DISPLAY_RESET;
#endif

#ifndef PIN_DISPLAY_BACKLIGHT
        constexpr uint8_t BACKLIGHT = 9;
#else
        constexpr uint8_t BACKLIGHT = PIN_DISPLAY_BACKLIGHT;
#endif
    }

    // SD Card specific
    namespace sd
    {
#ifndef PIN_SD_CS
        constexpr uint8_t CS = 34;
#else
        constexpr uint8_t CS = PIN_SD_CS;
#endif
    }

    // Analog inputs
    namespace analog
    {
#ifndef PIN_SOUND_SENSOR
        constexpr uint8_t SOUND = A0;
#else
        constexpr uint8_t SOUND = PIN_SOUND_SENSOR;
#endif
    }

    // LED Strip
    namespace led
    {
#ifndef PIN_LED_STRIP
        constexpr uint8_t STRIP = 21;
#else
        constexpr uint8_t STRIP = PIN_LED_STRIP;
#endif
    }
}

// Signal Processing Configuration
namespace signal_processing
{
    // EMA Configuration
    constexpr float EMA_ALPHA = 0.5f;       // Increased from 0.2 for much faster response
    constexpr float BASELINE_ALPHA = 0.01f; // Increased from 0.001 for faster baseline adaptation

    // Noise level ranges (adjusted to be more sensitive)
    namespace ranges
    {
        constexpr uint16_t QUIET = 500;    // Base level (green)
        constexpr uint16_t MODERATE = 700; // Lowered from 1000
        constexpr uint16_t LOUD = 1000;    // Lowered from 1500
        constexpr uint16_t MAX = 1300;     // Lowered from 1823
    }

    // Noise Categories (more sensitive thresholds)
    namespace thresholds
    {
        constexpr float NOISE_REGULAR = 1.1f; // 10% above baseline (was 1.2)
        constexpr float NOISE_HIGH = 1.3f;    // 30% above baseline (was 1.5)
        constexpr float NOISE_TOXIC = 1.5f;   // 50% above baseline (was 2.0)
    }
}

// Timing Configuration (milliseconds)
namespace intervals
{
    constexpr uint32_t SAMPLE_INTERVAL = 50;    // 50ms between readings
    constexpr uint32_t DISPLAY_INTERVAL = 1000; // 1s between display updates
    constexpr uint32_t LOG_INTERVAL = 60000;    // 1 minute between SD logs
}

// Display Configuration
namespace display_config
{
    // Plot settings
    namespace plot
    {
        constexpr uint8_t PLOT_POINTS = 60;              // 1 minute of data at current sample rate
        constexpr uint8_t PLOT_HEIGHT = 30;              // Plot height in pixels
        constexpr uint8_t PLOT_BASELINE_Y_POSITION = 63; // Plot baseline Y position
    }
}

// ADC Configuration
namespace adc_config
{
    constexpr uint8_t RESOLUTION_BITS = 12;
    constexpr uint16_t MAX_VALUE = (1 << RESOLUTION_BITS) - 1; // 4095 for 12-bit
    constexpr uint8_t AVERAGING_SAMPLES = 1;                   // Number of samples to average

    namespace sound_sensor
    {
        constexpr float VOLTAGE_REFERENCE = 3.3f; // ESP32 ADC reference voltage
        constexpr float VOLTAGE_GAIN = 26.0f;     // Grove sensor gain in dB
        constexpr float MIN_DB = -60.0f;          // Sensor min sensitivity in dBV/Pa
        constexpr float MAX_DB = -56.0f;          // Sensor max sensitivity in dBV/Pa
    }
}

// NeoPixel Configuration
namespace led_config
{
#ifndef LED_NUM_PIXELS
    constexpr uint8_t NUM_PIXELS = 8;
#else
    constexpr uint8_t NUM_PIXELS = LED_NUM_PIXELS;
#endif

    namespace colors
    {
        // Macro to calculate the color based on order (NEO_GRB)
        constexpr uint32_t make_color(uint8_t r, uint8_t g, uint8_t b)
        {
            return ((uint32_t)g << 16) | ((uint32_t)r << 8) | b;
        }

        constexpr uint32_t LEVEL_1 = make_color(0, 255, 0);   // Bright Green
        constexpr uint32_t LEVEL_2 = make_color(85, 255, 0);  // Yellowish Green
        constexpr uint32_t LEVEL_3 = make_color(170, 255, 0); // Lime Green
        constexpr uint32_t LEVEL_4 = make_color(255, 255, 0); // Yellow
        constexpr uint32_t LEVEL_5 = make_color(255, 170, 0); // Orange-Yellow
        constexpr uint32_t LEVEL_6 = make_color(255, 85, 0);  // Orange
        constexpr uint32_t LEVEL_7 = make_color(255, 0, 0);   // Bright Red
        constexpr uint32_t LEVEL_8 = make_color(128, 0, 0);   // Dark Red

        // Colors from green to red (explicitly ordered)
        constexpr uint32_t INDICATOR_COLORS[NUM_PIXELS] = {
            make_color(128, 0, 0),   // Dark Red
            make_color(255, 0, 0),   // Bright Red
            make_color(255, 85, 0),  // Orange
            make_color(255, 170, 0), // Orange-Yellow
            make_color(255, 255, 0), // Yellow
            make_color(170, 255, 0), // Yellow-Green
            make_color(85, 255, 0),  // Light Green
            make_color(0, 255, 0)    // Pure Green
        };
    }
}

// Alert Configuration
namespace alert
{
#ifndef SPEAKER_PIN
    constexpr uint8_t SPEAKER_PIN = 26;
#else
    constexpr uint8_t SPEAKER_PIN = PIN_SPEAKER;
#endif
    constexpr uint32_t ELEVATED_THRESHOLD_MS = 5000;     // 5 seconds
    constexpr uint32_t BEEP_DURATION_MS = 200;           // 200ms beep
    constexpr uint32_t BEEP_INTERVAL_MS = 1000;          // 1 second between beeps
    constexpr uint8_t MAX_ALERTS = 3;                    // Maximum number of alerts
    constexpr uint32_t BASE_COOLDOWN_MS = 10000;         // 10 second base cooldown
    constexpr uint32_t MAX_COOLDOWN_MS = 60000;          // Maximum 1 minute cooldown
    constexpr uint32_t RAPID_TRIGGER_WINDOW_MS = 120000; // 2 minute window to check for frequent triggers

    // Alarm sound parameters
    constexpr uint16_t ALARM_FREQUENCY = 2500;   // More piercing frequency
    constexpr uint16_t ALARM_FREQUENCY_2 = 3000; // Second frequency for alternating tone
}