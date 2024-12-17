#include "alert_manager.hpp"
#include "driver/dac.h"
#include <esp_log.h>

static const char* TAG = "AlertManager";

AlertManager::AlertManager() = default;

void AlertManager::begin() {
    pinMode(config::alert::SPEAKER_PIN, OUTPUT);
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 255);

    load_config();
}

void AlertManager::begin(const JsonObject& config) {
    pinMode(config::alert::SPEAKER_PIN, OUTPUT);
    dac_output_enable(DAC_CHANNEL_1);
    dac_output_voltage(DAC_CHANNEL_1, 255);

    if (!update_config(config)) {
        ESP_LOGW(TAG, "Invalid alert configuration, using defaults");
        load_config();
    }
}

void AlertManager::load_config() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_alert_config(config)) {
        update_config(config);
    } else {
        ESP_LOGW(TAG, "Failed to load alert configuration, using defaults");
        // Default values are already set in the header
    }
}

bool AlertManager::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    m_elevated_threshold_ms = config["elevated_threshold_ms"] | config::alert::ELEVATED_THRESHOLD_MS;
    m_beep_duration_ms = config["beep_duration_ms"] | config::alert::BEEP_DURATION_MS;
    m_beep_interval_ms = config["beep_interval_ms"] | config::alert::BEEP_INTERVAL_MS;
    m_max_alerts = config["max_alerts"] | config::alert::MAX_ALERTS;
    m_base_cooldown_ms = config["base_cooldown_ms"] | config::alert::BASE_COOLDOWN_MS;
    m_max_cooldown_ms = config["max_cooldown_ms"] | config::alert::MAX_COOLDOWN_MS;
    m_rapid_trigger_window_ms = config["rapid_trigger_window_ms"] | config::alert::RAPID_TRIGGER_WINDOW_MS;
    m_alarm_frequency = config["alarm_frequency"] | config::alert::ALARM_FREQUENCY;
    m_alarm_frequency_2 = config["alarm_frequency_2"] | config::alert::ALARM_FREQUENCY_2;

    return true;
}

bool AlertManager::validate_config(const JsonObject& config) {
    // Validate timing parameters
    if (config["elevated_threshold_ms"].is<uint32_t>() && 
        config["elevated_threshold_ms"].as<uint32_t>() < 100) {
        return false;
    }

    if (config["beep_duration_ms"].is<uint32_t>() && 
        config["beep_duration_ms"].as<uint32_t>() < 50) {
        return false;
    }

    if (config["beep_interval_ms"].is<uint32_t>() && 
        config["beep_interval_ms"].as<uint32_t>() < config["beep_duration_ms"].as<uint32_t>()) {
        return false;
    }

    if (config["max_alerts"].is<uint8_t>() && 
        config["max_alerts"].as<uint8_t>() == 0) {
        return false;
    }

    if (config["base_cooldown_ms"].is<uint32_t>() && 
        config["base_cooldown_ms"].as<uint32_t>() < 1000) {
        return false;
    }

    if (config["max_cooldown_ms"].is<uint32_t>() && 
        config["max_cooldown_ms"].as<uint32_t>() < config["base_cooldown_ms"].as<uint32_t>()) {
        return false;
    }

    // Validate frequencies
    if (config["alarm_frequency"].is<uint16_t>()) {
        uint16_t freq = config["alarm_frequency"].as<uint16_t>();
        if (freq < 100 || freq > 20000) {
            return false;
        }
    }

    if (config["alarm_frequency_2"].is<uint16_t>()) {
        uint16_t freq = config["alarm_frequency_2"].as<uint16_t>();
        if (freq < 100 || freq > 20000) {
            return false;
        }
    }

    return true;
}

void AlertManager::update(const SignalProcessor& signal_processor) {
    // First check if we're in cooldown
    if (m_in_cooldown) {
        if (check_cooldown()) {
            return; // Still in cooldown, don't process alerts
        }
    }

    bool is_currently_elevated = signal_processor.get_noise_category() >= SignalProcessor::NoiseLevel::ELEVATED;

    // Check for state changes
    if (is_currently_elevated && !m_is_elevated) {
        m_elevation_start_time = millis();
        m_is_elevated = true;
    }
    else if (!is_currently_elevated) {
        m_is_elevated = false;
        m_elevation_start_time = 0;
    }

    // If noise has been elevated for threshold duration, trigger alert
    if (m_is_elevated && 
        (millis() - m_elevation_start_time >= m_elevated_threshold_ms)) {
        trigger_alert();
    }
}

bool AlertManager::check_cooldown() {
    if (!m_in_cooldown) {
        return false;
    }

    if (millis() - m_cooldown_start_time >= m_base_cooldown_ms) {
        m_in_cooldown = false;
        return false;
    }

    return true;
}

void AlertManager::trigger_alert() {
    if (m_alert_count >= m_max_alerts) {
        start_cooldown();
        return;
    }

    // Check for rapid triggers
    unsigned long current_time = millis();
    if (current_time - m_last_trigger_time < m_rapid_trigger_window_ms) {
        m_rapid_trigger_count++;
    } else {
        m_rapid_trigger_count = 0;
    }
    m_last_trigger_time = current_time;

    // Play alert sound
    tone(config::alert::SPEAKER_PIN, m_alarm_frequency, m_beep_duration_ms);
    delay(m_beep_duration_ms);
    tone(config::alert::SPEAKER_PIN, m_alarm_frequency_2, m_beep_duration_ms);
    delay(m_beep_interval_ms);

    m_alert_count++;
    m_last_alert_time = current_time;

    if (m_rapid_trigger_count >= 3) {
        start_cooldown();
    }
}

void AlertManager::start_cooldown() {
    m_in_cooldown = true;
    m_cooldown_start_time = millis();
    m_alert_count = 0;
    m_rapid_trigger_count = 0;
}