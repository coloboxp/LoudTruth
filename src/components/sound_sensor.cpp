#include "sound_sensor.hpp"
#include <esp_log.h>

static const char* TAG = "SoundSensor";

SoundSensor::SoundSensor() {
    // Convert PIN_SOUND_SENSOR to ADC channel
    m_adc_channel = (adc1_channel_t)(PIN_SOUND_SENSOR - 36); // GPIO36 is ADC1_CHANNEL_0
}

void SoundSensor::begin() {
    load_config();
    init_adc();
    m_initialized = true;
}

void SoundSensor::begin(const JsonObject& config) {
    if (!update_config(config)) {
        ESP_LOGW(TAG, "Invalid sound sensor configuration, using defaults");
        load_config();
    }
    init_adc();
    m_initialized = true;
}

void SoundSensor::load_config() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_sound_sensor_config(config)) {
        update_config(config);
    } else {
        ESP_LOGW(TAG, "Failed to load sound sensor configuration, using defaults");
        // Default values are already set in the header
    }
}

bool SoundSensor::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    if (config["adc_width"].is<uint8_t>()) {
        uint8_t width = config["adc_width"].as<uint8_t>();
        switch (width) {
            case 9:  m_adc_width = ADC_WIDTH_BIT_9;  break;
            case 10: m_adc_width = ADC_WIDTH_BIT_10; break;
            case 11: m_adc_width = ADC_WIDTH_BIT_11; break;
            case 12: m_adc_width = ADC_WIDTH_BIT_12; break;
            default: return false;
        }
    }

    if (config["adc_attenuation"].is<uint8_t>()) {
        uint8_t atten = config["adc_attenuation"].as<uint8_t>();
        switch (atten) {
            case 0:  m_adc_attenuation = ADC_ATTEN_DB_0;   break;
            case 2:  m_adc_attenuation = ADC_ATTEN_DB_2_5; break;
            case 6:  m_adc_attenuation = ADC_ATTEN_DB_6;   break;
            case 11: m_adc_attenuation = ADC_ATTEN_DB_12;  break;
            default: return false;
        }
    }

    m_sampling_rate = config["sampling_rate"] | 1000;
    m_averaging_samples = config["averaging_samples"] | 4;
    m_voltage_reference = config["voltage_reference"] | 3.3f;
    m_calibration_offset = config["calibration_offset"] | 0.0f;
    m_calibration_scale = config["calibration_scale"] | 1.0f;

    if (m_initialized) {
        init_adc(); // Reinitialize ADC with new settings
    }

    return true;
}

bool SoundSensor::validate_config(const JsonObject& config) {
    if (config["sampling_rate"].is<uint16_t>()) {
        uint16_t rate = config["sampling_rate"].as<uint16_t>();
        if (rate < 100 || rate > 10000) {
            return false;
        }
    }

    if (config["averaging_samples"].is<uint8_t>()) {
        uint8_t samples = config["averaging_samples"].as<uint8_t>();
        if (samples < 1 || samples > 64) {
            return false;
        }
    }

    if (config["voltage_reference"].is<float>()) {
        float vref = config["voltage_reference"].as<float>();
        if (vref < 1.0f || vref > 3.9f) {
            return false;
        }
    }

    if (config["calibration_scale"].is<float>()) {
        float scale = config["calibration_scale"].as<float>();
        if (scale <= 0.0f || scale > 10.0f) {
            return false;
        }
    }

    return true;
}

void SoundSensor::init_adc() {
    // Configure ADC
    adc1_config_width(m_adc_width);
    adc1_config_channel_atten(m_adc_channel, m_adc_attenuation);
    
    // Characterize ADC
    esp_adc_cal_characterize(ADC_UNIT_1, m_adc_attenuation, m_adc_width, 
                            m_voltage_reference * 1000, &m_adc_chars);
}

uint16_t SoundSensor::read_raw() {
    if (!m_initialized) return 0;
    return read_averaged();
}

float SoundSensor::read_voltage() {
    if (!m_initialized) return 0.0f;
    
    uint16_t raw = read_averaged();
    uint32_t voltage_mv = esp_adc_cal_raw_to_voltage(raw, &m_adc_chars);
    return (voltage_mv / 1000.0f);
}

float SoundSensor::read_normalized() {
    if (!m_initialized) return 0.0f;
    
    float voltage = read_voltage();
    return (voltage + m_calibration_offset) * m_calibration_scale;
}

uint16_t SoundSensor::read_averaged() {
    uint32_t sum = 0;
    for (uint8_t i = 0; i < m_averaging_samples; i++) {
        sum += adc1_get_raw(m_adc_channel);
        if (m_averaging_samples > 1) {
            delayMicroseconds(1000000 / m_sampling_rate);
        }
    }
    return sum / m_averaging_samples;
}