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
    analogReadResolution(config::adc::RESOLUTION_BITS);

    // Set attenuation to 0db for maximum sensitivity
    analogSetAttenuation(ADC_0db);

    // Optimize ADC for fast sampling
    analogSetClockDiv(1); // Fastest ADC clock
    analogSetWidth(12);   // Ensure 12-bit resolution

    // Set pin-specific attenuation
    analogSetPinAttenuation(config::hardware::pins::analog::SOUND_SENSOR, ADC_0db);
}

/**
 * @brief Read an averaged sample from the sound sensor.
 * @param num_samples The number of samples to read and average.
 * @return The averaged sample value.
 */
uint16_t SoundSensor::read_averaged_sample(uint8_t num_samples)
{
    uint32_t sum = 0;
    uint16_t max_value = 0;

    // Take multiple rapid samples and track max
    for (uint8_t i = 0; i < num_samples; i++)
    {
        uint16_t sample = analogRead(config::hardware::pins::analog::SOUND_SENSOR);
        sum += sample;
        max_value = max(max_value, sample);
    }

    // Return the maximum value for better peak detection
    return max_value;
}