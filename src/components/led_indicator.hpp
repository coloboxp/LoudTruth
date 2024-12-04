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
    void update_level_display(float noise_level);
};