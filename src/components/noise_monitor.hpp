#pragma once

#include "sound_sensor.hpp"
#include "signal_processor.hpp"
#include "display_manager.hpp"
#include "led_indicator.hpp"
#include "alert_manager.hpp"
#include "data_logger.hpp"
#include "api_handler.hpp"
#include "api_server.hpp"
#include "config/configuration_manager.hpp"

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
    DisplayManager m_display_manager;
    LedIndicator m_led_indicator;
    AlertManager m_alert_manager;
    DataLogger m_logger;
    ApiServer &m_api_server{ApiServer::instance()};

    unsigned long m_last_sample_time{0};
    unsigned long m_last_display_time{0};
    unsigned long m_last_led_time{0};
    unsigned long m_last_log_time{0};
    unsigned long m_last_api_time{0};

    // Configuration cache
    uint32_t m_sample_interval{0};
    uint32_t m_display_interval{0};
    uint32_t m_logging_interval{0};
    uint32_t m_led_update_interval{0};

    bool load_configuration();
    void apply_configuration();
    void handle_sampling();
    void handle_display();
    void handle_logging();
    void handle_api_update();
};