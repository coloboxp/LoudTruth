#include "led_indicator.hpp"

/**
 * @brief Constructor for the LedIndicator class.
 */
LedIndicator::LedIndicator()
    : m_pixels(led_config::NUM_PIXELS,
               pins::led::STRIP,
               NEO_GRB + NEO_KHZ800)
{
}

/**
 * @brief Initialize the LED strip.
 */
void LedIndicator::begin()
{
    m_pixels.begin();
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
    // Calculate how many LEDs to light based on noise level
    float normalized_level = noise_level / adc_config::MAX_VALUE;
    int active_leds = static_cast<int>(normalized_level * led_config::NUM_PIXELS);

    // Ensure bounds
    active_leds = std::min(active_leds, static_cast<int>(led_config::NUM_PIXELS));
    active_leds = std::max(active_leds, 0);

    // Update LED colors
    for (int i = 0; i < led_config::NUM_PIXELS; i++)
    {
        if (i < active_leds)
        {
            m_pixels.setPixelColor(i, LEVEL_COLORS[i]);
        }
        else
        {
            m_pixels.setPixelColor(i, 0); // Turn off
        }
    }

    m_pixels.show();
}