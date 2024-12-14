#include "noise_monitor.hpp"

NoiseMonitor::NoiseMonitor() = default;

/**
 * @brief Initialize the noise monitor.
 * @return True if initialization is successful, false otherwise.
 */
bool NoiseMonitor::begin()
{
    m_sound_sensor.begin();
    m_display.begin();
    m_led_indicator.begin();
    m_alert_manager.begin();
    return m_logger.begin();
}

/**
 * @brief Update the noise monitor.
 */
void NoiseMonitor::update()
{
    unsigned long current_time = millis();

    // Handle periodic tasks
    handle_sampling();
    handle_display();
    handle_logging();

    // Update alert manager
    m_alert_manager.update(m_signal_processor);
}

/**
 * @brief Handle the sampling task.
 */
void NoiseMonitor::handle_sampling()
{
    unsigned long current_time = millis();

    if (current_time - m_last_sample_time >= intervals::SAMPLE_INTERVAL)
    {
        uint16_t raw_value = m_sound_sensor.read_averaged_sample();
        m_signal_processor.process_sample(raw_value);

        // Update plot buffer
        m_display.add_plot_point(m_signal_processor.get_current_value());

        m_last_sample_time = current_time;
    }
}

/**
 * @brief Handle the display task.
 */
void NoiseMonitor::handle_display()
{
    unsigned long current_time = millis();

    if (current_time - m_last_display_time >= intervals::DISPLAY_INTERVAL)
    {
        m_display.update(m_signal_processor);
        m_led_indicator.update(m_signal_processor);
        m_last_display_time = current_time;
    }
}

/**
 * @brief Handle the logging task.
 */
void NoiseMonitor::handle_logging()
{
    unsigned long current_time = millis();

    if (current_time - m_last_log_time >= intervals::LOG_INTERVAL)
    {
        m_logger.log_data(m_signal_processor);
        m_last_log_time = current_time;
    }
}