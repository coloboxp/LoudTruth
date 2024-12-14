#pragma once

#include "sound_sensor.hpp"
#include "signal_processor.hpp"
#include "display_manager.hpp"
#include "led_indicator.hpp"
#include "alert_manager.hpp"
#include "data_logger.hpp"
#include "api_handler.hpp"

/**
 * @brief Class representing the noise monitor.
 */
class NoiseMonitor
{
public:
    NoiseMonitor();
    bool begin();
    void update();

private:
    SoundSensor m_sound_sensor;
    SignalProcessor m_signal_processor;
    DisplayManager m_display;
    LedIndicator m_led_indicator;
    AlertManager m_alert_manager;
    DataLogger m_logger;

    unsigned long m_last_sample_time{0};
    unsigned long m_last_display_time{0};
    unsigned long m_last_led_time{0};
    unsigned long m_last_log_time{0};
    unsigned long m_last_api_time{0};

    void handle_sampling();
    void handle_display();
    void handle_logging();
    void handle_api_update();
};