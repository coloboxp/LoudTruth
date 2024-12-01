#pragma once

#include <Arduino.h>
#include "config/config.h"

class SoundSensor
{
public:
    SoundSensor() = default;
    void begin();

    // Takes multiple samples and returns their average
    uint16_t read_averaged_sample(uint8_t num_samples = adc_config::AVERAGING_SAMPLES);

private:
    void configure_adc();
};