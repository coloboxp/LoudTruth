#pragma once

#include <Arduino.h>
#include <driver/adc.h>
#include <esp_adc_cal.h>
#include "config/configuration_manager.hpp"

/**
 * @brief Class representing the sound sensor.
 */
class SoundSensor
{
public:
    SoundSensor();
    
    void begin();
    void begin(const JsonObject& config);
    bool update_config(const JsonObject& config);
    uint16_t read_raw();
    float read_voltage();
    float read_normalized();
    bool is_ready() const { return m_initialized; }
    uint16_t read() const { return adc1_get_raw(m_adc_channel); }

private:
    esp_adc_cal_characteristics_t m_adc_chars;
    bool m_initialized{false};
    
    // Configurable parameters
    adc1_channel_t m_adc_channel{ADC1_CHANNEL_0};
    adc_bits_width_t m_adc_width{ADC_WIDTH_BIT_12};
    adc_atten_t m_adc_attenuation{ADC_ATTEN_DB_12};
    uint16_t m_sampling_rate{1000};
    uint8_t m_averaging_samples{4};
    float m_voltage_reference{3.3f};
    float m_calibration_offset{0.0f};
    float m_calibration_scale{1.0f};
    
    void load_config();
    bool validate_config(const JsonObject& config);
    void init_adc();
    uint16_t read_averaged();
};