#pragma once
#include <string>
#include <WiFi.h>

// Try to include private configuration if available
#if __has_include("private.h")
#include "private.h"
#endif

namespace config
{
    namespace wifi
    {
#ifndef WIFI_SSID
        constexpr char const *SSID = "your_ssid";
#else
        constexpr char const *SSID = WIFI_SSID;
#endif

#ifndef WIFI_PASS
        constexpr char const *PASSWORD = "your_password";
#else
        constexpr char const *PASSWORD = WIFI_PASS;
#endif
    }

    namespace hardware
    {
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

            namespace display
            {
                constexpr uint8_t MOSI = pins::MOSI;
                constexpr uint8_t SCK = pins::SCK;

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

            namespace sd
            {
#ifndef PIN_SD_CS
                constexpr uint8_t CS = 34;
#else
                constexpr uint8_t CS = PIN_SD_CS;
#endif
            }

            namespace analog
            {
#ifndef PIN_SOUND_SENSOR
                constexpr uint8_t SOUND_SENSOR = A0;
#else
                constexpr uint8_t SOUND_SENSOR = PIN_SOUND_SENSOR;
#endif
            }

            namespace led
            {
#ifndef PIN_LED_STRIP
                constexpr uint8_t STRIP = 21;
#else
                constexpr uint8_t STRIP = PIN_LED_STRIP;
#endif

                constexpr uint8_t DATA = 2;  // Add missing DATA pin definition
            }

            // Add LED indicator pin definition
            constexpr uint8_t LED_INDICATOR = 2; // Built-in LED pin

            // Sound sensor is in the analog namespace, so we'll create an alias
            constexpr uint8_t SOUND_SENSOR = analog::SOUND_SENSOR;
        }
    }

    namespace signal_processing
    {
        // Optimized for peak-to-peak measurements
        constexpr float EMA_ALPHA = 0.7f;         // Balanced response
        constexpr float BASELINE_ALPHA = 0.0005f; // Very slow baseline adjustment

        namespace ranges
        {
            // Adjusted for peak-to-peak values
            constexpr uint16_t QUIET = 50;     // Was 100
            constexpr uint16_t MODERATE = 150; // Was 300
            constexpr uint16_t LOUD = 300;     // Was 500
            constexpr uint16_t MAX = 500;      // Was 800
        }

        namespace thresholds
        {
            constexpr float NOISE_REGULAR = 1.2f;
            constexpr float NOISE_HIGH = 1.4f;
            constexpr float NOISE_TOXIC = 1.6f;
        }
    }

    namespace timing
    {
        struct TimingConfig
        {
            static uint32_t sample_interval;
            static uint32_t display_interval;
            static uint32_t logging_interval;
            static uint32_t led_update_interval;

            static constexpr uint32_t DEFAULT_SAMPLE_INTERVAL = 100;
            static constexpr uint32_t DEFAULT_DISPLAY_INTERVAL = 1000;
            static constexpr uint32_t DEFAULT_LOGGING_INTERVAL = 60000;
            static constexpr uint32_t DEFAULT_LED_UPDATE_INTERVAL = 50;
        };

        // Use static constexpr references instead of inline variables
        static constexpr const uint32_t &SAMPLE_INTERVAL = TimingConfig::sample_interval;
        static constexpr const uint32_t &DISPLAY_INTERVAL = TimingConfig::display_interval;
        static constexpr const uint32_t &LOGGING_INTERVAL = TimingConfig::logging_interval;
        static constexpr const uint32_t &LED_UPDATE_INTERVAL = TimingConfig::led_update_interval;
    }

    namespace display
    {
        namespace plot
        {
            constexpr uint8_t PLOT_POINTS = 60;
            constexpr uint8_t PLOT_HEIGHT = 30;
            constexpr uint8_t PLOT_BASELINE_Y_POSITION = 63;
        }
    }

    namespace adc
    {
        constexpr uint8_t RESOLUTION_BITS = 12;
        constexpr uint16_t MAX_VALUE = (1 << RESOLUTION_BITS) - 1;
        constexpr uint8_t AVERAGING_SAMPLES = 1; // Back to 1 for fastest response

        namespace sound_sensor
        {
            constexpr float VOLTAGE_REFERENCE = 3.3f;
            constexpr float VOLTAGE_GAIN = 52.0f;
            constexpr float MIN_DB = -60.0f;
            constexpr float MAX_DB = -56.0f;
        }
    }

    namespace led
    {
        // Color helper function
        constexpr uint32_t make_color(uint8_t r, uint8_t g, uint8_t b)
        {
            return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
        }

#ifndef LED_NUM_PIXELS
        constexpr uint8_t NUM_PIXELS = 8;
#else
        constexpr uint8_t NUM_PIXELS = LED_NUM_PIXELS;
#endif

        namespace colors
        {
            constexpr uint32_t INDICATOR_COLORS[NUM_PIXELS] = {
                make_color(0, 255, 0),   // Pure Green
                make_color(85, 255, 0),  // Light Green
                make_color(170, 255, 0), // Yellow-Green
                make_color(255, 255, 0), // Yellow
                make_color(255, 170, 0), // Orange-Yellow
                make_color(255, 85, 0),  // Orange
                make_color(255, 0, 0),   // Bright Red
                make_color(128, 0, 0)    // Dark Red
            };
        }
    }

    namespace alert
    {
#ifndef SPEAKER_PIN
        constexpr uint8_t SPEAKER_PIN = 26;
#else
        constexpr uint8_t SPEAKER_PIN = PIN_SPEAKER;
#endif
        constexpr uint32_t ELEVATED_THRESHOLD_MS = 5000;
        constexpr uint32_t BEEP_DURATION_MS = 200;
        constexpr uint32_t BEEP_INTERVAL_MS = 1000;
        constexpr uint8_t MAX_ALERTS = 3;
        constexpr uint32_t BASE_COOLDOWN_MS = 10000;
        constexpr uint32_t MAX_COOLDOWN_MS = 60000;
        constexpr uint32_t RAPID_TRIGGER_WINDOW_MS = 120000;
        constexpr uint16_t ALARM_FREQUENCY = 2500;
        constexpr uint16_t ALARM_FREQUENCY_2 = 3000;
    }

    namespace ntp
    {
        constexpr char const *SERVER = "pool.ntp.org";
        constexpr char const *TIMEZONE = "CET-1CEST,M3.5.0,M10.5.0/3";
        constexpr long GMT_OFFSET_SEC = 3600;
        constexpr long DAYLIGHT_OFFSET_SEC = 3600;
        constexpr unsigned long SYNC_INTERVAL = 3600000;
    }

    namespace thingspeak
    {
        constexpr char const *ENDPOINT = "https://api.thingspeak.com";

#ifndef THINGSPEAK_USER_API_KEY
        constexpr char const *USER_API_KEY = "your_user_api_key";
#else
        constexpr char const *USER_API_KEY = THINGSPEAK_USER_API_KEY;
#endif

#ifndef THINGSPEAK_NOISE_API_KEY
        constexpr char const *NOISE_API_KEY = "your_alerts_api_key";
#else
        constexpr char const *NOISE_API_KEY = THINGSPEAK_NOISE_API_KEY;
#endif

        constexpr unsigned long UPDATE_INTERVAL_MS = 30000;
        constexpr int MAX_FIELDS = 8;
        constexpr bool PUBLIC_FLAG = false;

#ifndef THINGSPEAK_NOISE_CHANNEL_ID
        constexpr char const *NOISE_CHANNEL_ID = "your_noise_channel_id";
#else
        constexpr char const *NOISE_CHANNEL_ID = THINGSPEAK_NOISE_CHANNEL_ID;
#endif
    }

    namespace device
    {
        constexpr char const *NAME = "NoiseMonitor";
    }

    namespace signal_processing
    {
        namespace ranges
        {
            extern int quiet;
            extern int moderate;
            extern int loud;
            extern int max;
        }
    }
}