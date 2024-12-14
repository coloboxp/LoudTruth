#include "alert_manager.hpp"
#include "driver/dac.h"

AlertManager::AlertManager() = default;

void AlertManager::begin()
{
    pinMode(config::alert::SPEAKER_PIN, OUTPUT);
    // Initialize DAC for maximum amplitude
    dac_output_enable(DAC_CHANNEL_1);       // GPIO26 is DAC channel 1
    dac_output_voltage(DAC_CHANNEL_1, 255); // Set to maximum voltage
}

void AlertManager::update(const SignalProcessor &signal_processor)
{
    // First check if we're in cooldown
    if (m_in_cooldown)
    {
        if (check_cooldown())
        {
            return; // Still in cooldown, don't process alerts
        }
    }

    bool is_currently_elevated = signal_processor.get_noise_category() >= SignalProcessor::NoiseLevel::ELEVATED;

    // Check for state changes
    if (is_currently_elevated && !m_is_elevated)
    {
        m_elevation_start_time = millis();
        m_is_elevated = true;
    }
    else if (!is_currently_elevated)
    {
        m_is_elevated = false;
        m_elevation_start_time = 0;
    }

    // If noise has been elevated for threshold duration, handle alert
    if (m_is_elevated &&
        (millis() - m_elevation_start_time >= config::alert::ELEVATED_THRESHOLD_MS))
    {
        handle_alert();
    }
}

void AlertManager::handle_alert()
{
    if (m_alert_count >= config::alert::MAX_ALERTS)
    {
        start_cooldown();
        return;
    }

    unsigned long current_time = millis();

    if (current_time - m_last_beep_time >= config::alert::BEEP_INTERVAL_MS)
    {
        // Number of beeps increases with rapid triggers
        int num_beeps = (m_rapid_trigger_count > 0) ? 8 : 4;

        // Generate maximum amplitude square wave
        for (int i = 0; i < num_beeps; i++)
        {
            for (int j = 0; j < config::alert::ALARM_FREQUENCY / 10; j++)
            { // Generate for 100ms
                digitalWrite(config::alert::SPEAKER_PIN, HIGH);
                delayMicroseconds(500000 / config::alert::ALARM_FREQUENCY);
                digitalWrite(config::alert::SPEAKER_PIN, LOW);
                delayMicroseconds(500000 / config::alert::ALARM_FREQUENCY);
            }
            delay(50); // Gap between beeps
        }

        m_last_beep_time = current_time;
        m_alert_count++;

        if (m_alert_count >= config::alert::MAX_ALERTS)
        {
            start_cooldown();
        }
    }
}

void AlertManager::start_cooldown()
{
    m_in_cooldown = true;
    m_cooldown_start_time = millis();
    m_alert_count = 0;
    update_cooldown_duration();
    m_last_trigger_time = millis();
}

void AlertManager::update_cooldown_duration()
{
    unsigned long current_time = millis();

    if (m_last_trigger_time > 0 &&
        (current_time - m_last_trigger_time) < config::alert::RAPID_TRIGGER_WINDOW_MS)
    {
        m_rapid_trigger_count++;
        m_current_cooldown_ms = std::min(
            config::alert::MAX_COOLDOWN_MS,
            config::alert::BASE_COOLDOWN_MS * (m_rapid_trigger_count + 1));
    }
    else
    {
        m_rapid_trigger_count = 0;
        m_current_cooldown_ms = config::alert::BASE_COOLDOWN_MS;
    }
}

bool AlertManager::check_cooldown()
{
    if (!m_in_cooldown)
    {
        return false;
    }

    if (millis() - m_cooldown_start_time >= m_current_cooldown_ms)
    {
        m_in_cooldown = false;
        return false;
    }

    return true;
}