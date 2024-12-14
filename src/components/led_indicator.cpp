#include "led_indicator.hpp"

/**
 * @brief Constructor for the LedIndicator class.
 */
LedIndicator::LedIndicator()
    : m_pixels(config::led::NUM_PIXELS,
               config::hardware::pins::led::STRIP,
               NEO_GRB + NEO_KHZ800)
{
}

/**
 * @brief Initialize the LED strip.
 */
void LedIndicator::begin()
{
    m_pixels.begin();
    m_pixels.setBrightness(32);
    m_pixels.clear();
    m_pixels.show();
}

/**
 * @brief Update the LED display based on the noise level.
 * @param signal_processor The signal processor instance.
 */
void LedIndicator::update(const SignalProcessor &signal_processor)
{
    float current_level = signal_processor.get_current_value();
    update_level_display(current_level);
}

/**
 * @brief Update the LED display based on the noise level.
 * @param noise_level The current noise level.
 */
void LedIndicator::update_level_display(float noise_level)
{
    // More sensitive thresholds for each LED
    constexpr uint16_t thresholds[config::led::NUM_PIXELS] = {
        config::signal_processing::ranges::QUIET,          // LED 0 (green) - 500
        config::signal_processing::ranges::QUIET + 100,    // LED 1 - 600
        config::signal_processing::ranges::MODERATE,       // LED 2 - 700
        config::signal_processing::ranges::MODERATE + 100, // LED 3 - 800
        config::signal_processing::ranges::MODERATE + 200, // LED 4 - 900
        config::signal_processing::ranges::LOUD,           // LED 5 - 1000
        config::signal_processing::ranges::LOUD + 150,     // LED 6 - 1150
        config::signal_processing::ranges::MAX             // LED 7 - 1300
    };

    // Calculate how many LEDs should be on, starting from LED 0
    int active_leds = 1; // Always show at least the first (green) LED for normal levels
    for (int i = 1; i < config::led::NUM_PIXELS; i++)
    {
        if (noise_level >= thresholds[i])
        {
            active_leds = i + 1;
        }
    }

    // Update LED colors - normal order, starting from 0
    for (int i = 0; i < config::led::NUM_PIXELS; i++)
    {
        if (i < active_leds)
        {
            m_pixels.setPixelColor(i, config::led::colors::INDICATOR_COLORS[i]);
        }
        else
        {
            m_pixels.setPixelColor(i, 0); // Turn off
        }
    }

    m_pixels.show();
}