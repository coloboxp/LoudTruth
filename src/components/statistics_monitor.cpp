#include "statistics_monitor.hpp"

StatisticsMonitor::StatisticsMonitor(const Config &config)
    : m_config(config)
{
    reset();
}

void StatisticsMonitor::reset()
{
    m_stats = Stats{};
    m_stats.history.clear();
}

void StatisticsMonitor::update(float value)
{
    unsigned long current_time = millis();

    // Initialize if this is the first update
    if (m_stats.last_update == 0)
    {
        m_stats.min = m_stats.max = m_stats.avg = value;
        m_stats.last_update = current_time;
    }

    // Update current value and extremes
    m_stats.current = value;
    m_stats.min = std::min(m_stats.min, value);
    m_stats.max = std::max(m_stats.max, value);

    // Update rolling average and history
    update_rolling_average(value);
    update_history(value);

    m_stats.last_update = current_time;
    m_stats.samples++;
}

void StatisticsMonitor::update_rolling_average(float value)
{
    // Use exponential moving average for rolling average
    constexpr float alpha = 0.1f; // Smoothing factor
    m_stats.avg = (alpha * value) + ((1.0f - alpha) * m_stats.avg);
}

void StatisticsMonitor::update_history(float value)
{
    m_stats.history.push_back(value);
    while (m_stats.history.size() > m_config.history_size)
    {
        m_stats.history.pop_front();
    }
}