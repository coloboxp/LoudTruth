#pragma once

#include <Arduino.h>

// Hardware Configuration
namespace pins
{
    // SPI Bus (shared between display and SD)
    constexpr uint8_t MOSI = 11;
    constexpr uint8_t MISO = 13;
    constexpr uint8_t SCK = 10;

    // Display specific (ST7565 ERC12864)
    namespace display
    {
        constexpr uint8_t CS = 19;
        constexpr uint8_t DC = 21;
        constexpr uint8_t RESET = 20;
        constexpr uint8_t BACKLIGHT = 9;
    }

    // SD Card specific
    namespace sd
    {
        constexpr uint8_t CS = 34;
    }

    // Analog inputs
    namespace analog
    {
        constexpr uint8_t SOUND = A0;
    }
}

// Signal Processing Configuration
namespace signal_processing
{
    // EMA Configuration
    constexpr float EMA_ALPHA = 0.1f;        // Fast response for current noise
    constexpr float BASELINE_ALPHA = 0.001f; // Slow response for baseline

    // Noise Categories (multipliers above baseline)
    namespace thresholds
    {
        constexpr float NOISE_REGULAR = 1.5f; // 50% above baseline
        constexpr float NOISE_HIGH = 2.0f;    // 100% above baseline
        constexpr float NOISE_TOXIC = 3.0f;   // 200% above baseline
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
    constexpr uint8_t AVERAGING_SAMPLES = 32;                  // Number of samples to average

    namespace sound_sensor
    {
        constexpr float VOLTAGE_REFERENCE = 3.3f; // ESP32 ADC reference voltage
        constexpr float VOLTAGE_GAIN = 26.0f;     // Grove sensor gain in dB
        constexpr float MIN_DB = -60.0f;          // Sensor min sensitivity in dBV/Pa
        constexpr float MAX_DB = -56.0f;          // Sensor max sensitivity in dBV/Pa
    }
}