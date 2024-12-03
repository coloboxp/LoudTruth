#pragma once

#include <Adafruit_NeoPixel.h>
#include "config/config.h"
#include "signal_processor.hpp"

class LedIndicator
{
public:
    LedIndicator();
    void begin();
    void update(const SignalProcessor &signal_processor);

private:
    Adafruit_NeoPixel m_pixels;
    static constexpr uint32_t LEVEL_COLORS[led_config::NUM_PIXELS] = {
        led_config::colors::LEVEL_1,
        led_config::colors::LEVEL_2,
        led_config::colors::LEVEL_3,
        led_config::colors::LEVEL_4,
        led_config::colors::LEVEL_5,
        led_config::colors::LEVEL_6,
        led_config::colors::LEVEL_7,
        led_config::colors::LEVEL_8};

    void update_level_display(float noise_level);
};