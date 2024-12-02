#include "sound_sensor.hpp"

/**
 * @brief Initialize the sound sensor.
 */
void SoundSensor::begin()
{
    configure_adc();
}

/**
 * @brief Configure the ADC for the sound sensor.
 */
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

/**
 * @brief Read an averaged sample from the sound sensor.
 * @param num_samples The number of samples to read and average.
 * @return The averaged sample value.
 */
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