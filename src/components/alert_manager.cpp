#include "alert_manager.hpp"
#include "driver/dac.h"

AlertManager::AlertManager() = default;

void AlertManager::begin()
{
    pinMode(alert::SPEAKER_PIN, OUTPUT);
    // Initialize DAC for maximum amplitude
    dac_output_enable(DAC_CHANNEL_1);  // GPIO26 is DAC channel 1
    dac_output_voltage(DAC_CHANNEL_1, 255);  // Set to maximum voltage
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
        // Noise just became elevated
        m_elevation_start_time = millis();
        m_is_elevated = true;
    }
    else if (!is_currently_elevated)
    {
        // Reset when noise returns to normal
        m_is_elevated = false;
        m_elevation_start_time = 0;
    }

    // If noise has been elevated for threshold duration, handle alert
    if (m_is_elevated &&
        (millis() - m_elevation_start_time >= alert::ELEVATED_THRESHOLD_MS))
    {
        handle_alert();
    }
}

void AlertManager::handle_alert()
{
    if (m_alert_count >= alert::MAX_ALERTS) {
        start_cooldown();
        return;
    }

    unsigned long current_time = millis();
    
    if (current_time - m_last_beep_time >= alert::BEEP_INTERVAL_MS) {
        // Number of beeps increases with rapid triggers
        int num_beeps = (m_rapid_trigger_count > 0) ? 8 : 4;
        
        // Generate maximum amplitude square wave
        for(int i = 0; i < num_beeps; i++) {
            for(int j = 0; j < alert::ALARM_FREQUENCY/10; j++) {  // Generate for 100ms
                digitalWrite(alert::SPEAKER_PIN, HIGH);  // Full voltage
                delayMicroseconds(500000/alert::ALARM_FREQUENCY);  // Half period
                digitalWrite(alert::SPEAKER_PIN, LOW);   // Zero voltage
                delayMicroseconds(500000/alert::ALARM_FREQUENCY);  // Half period
            }
            delay(50);  // Gap between beeps
        }
        
        m_last_beep_time = current_time;
        m_alert_count++;

        if (m_alert_count >= alert::MAX_ALERTS) {
            start_cooldown();
        }
    }
}

void AlertManager::start_cooldown()
{
    m_in_cooldown = true;
    m_cooldown_start_time = millis();
    m_alert_count = 0;

    // Update cooldown duration based on trigger frequency
    update_cooldown_duration();

    // Record this trigger time
    m_last_trigger_time = millis();
}

void AlertManager::update_cooldown_duration()
{
    unsigned long current_time = millis();

    // Check if this trigger is within the rapid trigger window
    if (m_last_trigger_time > 0 &&
        (current_time - m_last_trigger_time) < alert::RAPID_TRIGGER_WINDOW_MS)
    {
        // Increase rapid trigger count
        m_rapid_trigger_count++;

        // Increase cooldown duration
        m_current_cooldown_ms = std::min(
            alert::MAX_COOLDOWN_MS,
            alert::BASE_COOLDOWN_MS * (m_rapid_trigger_count + 1));
    }
    else
    {
        // Reset if outside rapid trigger window
        m_rapid_trigger_count = 0;
        m_current_cooldown_ms = alert::BASE_COOLDOWN_MS;
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
        // Cooldown period is over
        m_in_cooldown = false;
        return false;
    }

    return true; // Still in cooldown
}