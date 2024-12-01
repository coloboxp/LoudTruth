#include "sound_sensor.hpp"

void SoundSensor::begin()
{
    // Configure ADC if needed
    analogReadResolution(12); // Set to 12-bit resolution for ESP32
}

uint16_t SoundSensor::read_averaged_sample(uint8_t num_samples)
{
    long sum = 0;

    // Read multiple samples and average them
    for (uint8_t i = 0; i < num_samples; i++)
    {
        sum += analogRead(pins::analog::SOUND);
    }

    return sum / num_samples;
}