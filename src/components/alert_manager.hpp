#pragma once
#include "config/config.h"
#include "signal_processor.hpp"

class AlertManager
{
public:
    AlertManager();
    void begin();
    void update(const SignalProcessor &signal_processor);

private:
    bool m_is_elevated{false};
    bool m_in_cooldown{false};
    uint8_t m_alert_count{0};
    uint8_t m_rapid_trigger_count{0};
    unsigned long m_elevation_start_time{0};
    unsigned long m_last_beep_time{0};
    unsigned long m_cooldown_start_time{0};
    unsigned long m_last_trigger_time{0};
    uint32_t m_current_cooldown_ms{config::alert::BASE_COOLDOWN_MS};

    void handle_alert();
    void start_cooldown();
    bool check_cooldown();
    void update_cooldown_duration();
};