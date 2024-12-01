#pragma once

#include <Arduino.h>
#include "config/config.h"

class SoundSensor
{
public:
    SoundSensor() = default;
    void begin();

    // Takes multiple samples and returns their average
    uint16_t read_averaged_sample(uint8_t num_samples = 32);

private:
    static constexpr uint8_t DEFAULT_SAMPLES = 32;
};