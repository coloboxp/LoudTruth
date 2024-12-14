#pragma once

#include <Arduino.h>
#include "config/config.h"

/**
 * @brief Class representing the sound sensor.
 */
class SoundSensor
{
public:
    SoundSensor() = default;
    void begin();

    // Takes multiple samples and returns their average
    uint16_t read_averaged_sample(uint8_t num_samples = config::adc::AVERAGING_SAMPLES);

private:
    void configure_adc();
};