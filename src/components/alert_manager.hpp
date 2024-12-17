#pragma once

#include <Arduino.h>
#include "signal_processor.hpp"
#include "config/configuration_manager.hpp"

class AlertManager
{
public:
    AlertManager();
    
    void begin();
    void begin(const JsonObject& config);
    bool update_config(const JsonObject& config);
    void update(const SignalProcessor& signal_processor);

private:
    bool m_in_cooldown{false};
    bool m_is_elevated{false};
    uint32_t m_elevation_start_time{0};
    uint32_t m_last_alert_time{0};
    uint32_t m_cooldown_start_time{0};
    uint32_t m_alert_count{0};
    uint32_t m_rapid_trigger_count{0};
    uint32_t m_last_trigger_time{0};

    // Configurable parameters
    uint32_t m_elevated_threshold_ms{config::alert::ELEVATED_THRESHOLD_MS};
    uint32_t m_beep_duration_ms{config::alert::BEEP_DURATION_MS};
    uint32_t m_beep_interval_ms{config::alert::BEEP_INTERVAL_MS};
    uint8_t m_max_alerts{config::alert::MAX_ALERTS};
    uint32_t m_base_cooldown_ms{config::alert::BASE_COOLDOWN_MS};
    uint32_t m_max_cooldown_ms{config::alert::MAX_COOLDOWN_MS};
    uint32_t m_rapid_trigger_window_ms{config::alert::RAPID_TRIGGER_WINDOW_MS};
    uint16_t m_alarm_frequency{config::alert::ALARM_FREQUENCY};
    uint16_t m_alarm_frequency_2{config::alert::ALARM_FREQUENCY_2};

    bool check_cooldown();
    void trigger_alert();
    void start_cooldown();
    bool validate_config(const JsonObject& config);
    void load_config();
};