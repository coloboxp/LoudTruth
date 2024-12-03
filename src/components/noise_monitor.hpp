#pragma once

#include "sound_sensor.hpp"
#include "display_manager.hpp"
#include "data_logger.hpp"
#include "signal_processor.hpp"
#include "led_indicator.hpp"
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
    DisplayManager m_display;
    DataLogger m_logger;
    SignalProcessor m_signal_processor;
    LedIndicator m_led_indicator;

    unsigned long m_last_sample_time{0};
    unsigned long m_last_display_time{0};
    unsigned long m_last_log_time{0};

    void handle_sampling();
    void handle_display();
    void handle_logging();
};