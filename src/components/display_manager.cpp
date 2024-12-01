#include "display_manager.hpp"

DisplayManager::DisplayManager()
    : m_u8g2(U8G2_R0,
             pins::display::CS,
             pins::display::DC,
             pins::display::RESET)
{
    memset(m_plot_buffer, 0, sizeof(m_plot_buffer));
}

void DisplayManager::begin()
{
    m_u8g2.begin();
    pinMode(pins::display::BACKLIGHT, OUTPUT);
    digitalWrite(pins::display::BACKLIGHT, HIGH);
}

void DisplayManager::update(const SignalProcessor &signal_processor)
{
    m_u8g2.clearBuffer();
    draw_stats(signal_processor);
    draw_plot();
    m_u8g2.sendBuffer();
}

void DisplayManager::add_plot_point(int value)
{
    m_plot_buffer[m_plot_index] = value;
    m_plot_index = (m_plot_index + 1) % display_config::plot::PLOT_POINTS;
}

void DisplayManager::draw_stats(const SignalProcessor &signal_processor)
{
    m_u8g2.setFont(u8g2_font_6x10_tf);

    // Draw noise level and category
    String noise_str = "Noise: " + String((int)signal_processor.get_current_value());
    m_u8g2.drawStr(0, 10, noise_str.c_str());
    m_u8g2.drawStr(64, 10,
                   noise_level_to_string(signal_processor.get_noise_category()));

    // Draw statistics
    const auto &one_min = signal_processor.get_one_min_stats();
    const auto &fifteen_min = signal_processor.get_fifteen_min_stats();

    String one_min_str = "1m:  " + String((int)one_min.avg) +
                         " [" + String(one_min.min) + "-" + String(one_min.max) + "]";
    String fifteen_min_str = "15m: " + String((int)fifteen_min.avg) +
                             " [" + String(fifteen_min.min) + "-" + String(fifteen_min.max) + "]";

    m_u8g2.drawStr(0, 20, one_min_str.c_str());
    m_u8g2.drawStr(0, 30, fifteen_min_str.c_str());
}

void DisplayManager::draw_plot()
{
    for (int i = 0; i < display_config::plot::PLOT_POINTS - 1; i++)
    {
        int x1 = i * 2;
        int x2 = (i + 1) * 2;
        int y1 = display_config::plot::PLOT_BASELINE_Y_POSITION -
                 map(m_plot_buffer[i], 0, adc_config::MAX_VALUE, 0,
                     display_config::plot::PLOT_HEIGHT);
        int y2 = display_config::plot::PLOT_BASELINE_Y_POSITION -
                 map(m_plot_buffer[(i + 1)], 0, adc_config::MAX_VALUE, 0,
                     display_config::plot::PLOT_HEIGHT);
        m_u8g2.drawLine(x1, y1, x2, y2);
    }
}

const char *DisplayManager::noise_level_to_string(SignalProcessor::NoiseLevel level)
{
    switch (level)
    {
    case SignalProcessor::NoiseLevel::OK:
        return "OK";
    case SignalProcessor::NoiseLevel::REGULAR:
        return "REGULAR";
    case SignalProcessor::NoiseLevel::ELEVATED:
        return "ELEVATED";
    case SignalProcessor::NoiseLevel::CRITICAL:
        return "CRITICAL";
    default:
        return "UNKNOWN";
    }
}