#include "sound_sensor.hpp"

void SoundSensor::begin()
{
    configure_adc();
}

void SoundSensor::configure_adc()
{
    // Configure ADC for 12-bit resolution
    analogReadResolution(adc_config::RESOLUTION_BITS);

    // Set attenuation for 3.3V max voltage
    analogSetAttenuation(ADC_11db);

// Enable ADC calibration if available
#if defined(ESP32)
    analogSetClockDiv(1); // Set ADC clock divider
#endif
}

uint16_t SoundSensor::read_averaged_sample(uint8_t num_samples)
{
    uint32_t sum = 0;

    // Read multiple samples and average them
    for (uint8_t i = 0; i < num_samples; i++)
    {
        sum += analogRead(pins::analog::SOUND);
    }

    return sum / num_samples;
}