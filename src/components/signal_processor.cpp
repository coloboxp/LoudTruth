#include "signal_processor.hpp"

SignalProcessor::SignalProcessor()
{
    // Add default monitors with explicit Config objects
    StatisticsMonitor::Config one_min_config;
    one_min_config.id = "1min";
    one_min_config.label = "1 Minute";
    one_min_config.period_ms = 60000;
    one_min_config.priority = 1;
    add_monitor(one_min_config);

    StatisticsMonitor::Config fifteen_min_config;
    fifteen_min_config.id = "15min";
    fifteen_min_config.label = "15 Minutes";
    fifteen_min_config.period_ms = 900000;
    fifteen_min_config.priority = 2;
    add_monitor(fifteen_min_config);
}

void SignalProcessor::process_sample(uint16_t raw_value)
{
    update_ema(raw_value);
    update_monitors(m_ema_value);
}

void SignalProcessor::update_ema(uint16_t raw_value)
{
    // Update current EMA
    constexpr float alpha = 0.1f;
    m_ema_value = (alpha * raw_value) + ((1.0f - alpha) * m_ema_value);

    // Update baseline EMA more slowly
    constexpr float baseline_alpha = 0.001f;
    m_baseline_ema = (baseline_alpha * raw_value) + ((1.0f - baseline_alpha) * m_baseline_ema);
}

void SignalProcessor::update_monitors(float value)
{
    for (auto &monitor : m_monitors)
    {
        monitor->update(value);
    }
}

SignalProcessor::NoiseLevel SignalProcessor::get_noise_category() const
{
    float ratio = m_ema_value / m_baseline_ema;

    if (ratio < 1.1f)
        return NoiseLevel::OK;
    if (ratio < 1.5f)
        return NoiseLevel::REGULAR;
    if (ratio < 2.0f)
        return NoiseLevel::ELEVATED;
    return NoiseLevel::CRITICAL;
}

void SignalProcessor::add_monitor(const StatisticsMonitor::Config &config)
{
    // Check if monitor with this ID already exists
    for (const auto &monitor : m_monitors)
    {
        if (monitor->get_id() == config.id)
        {
            return; // Already exists
        }
    }

    // Create new monitor
    m_monitors.push_back(std::unique_ptr<StatisticsMonitor>(new StatisticsMonitor(config)));
}

void SignalProcessor::remove_monitor(const std::string &id)
{
    auto it = m_monitors.begin();
    while (it != m_monitors.end())
    {
        if ((*it)->get_id() == id)
        {
            it = m_monitors.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

StatisticsMonitor *SignalProcessor::get_monitor(const std::string &id) const
{
    for (const auto &monitor : m_monitors)
    {
        if (monitor->get_id() == id)
        {
            return monitor.get();
        }
    }
    return nullptr;
}

std::vector<StatisticsMonitor *> SignalProcessor::get_priority_monitors(size_t count) const
{
    std::vector<StatisticsMonitor *> result;
    std::vector<StatisticsMonitor *> sorted_monitors;

    // Create vector of raw pointers
    for (const auto &monitor : m_monitors)
    {
        sorted_monitors.push_back(monitor.get());
    }

    // Sort by priority
    std::sort(sorted_monitors.begin(), sorted_monitors.end(),
              [](const StatisticsMonitor *a, const StatisticsMonitor *b)
              {
                  return a->get_priority() < b->get_priority();
              });

    // Return top N monitors
    size_t n = std::min(count, sorted_monitors.size());
    result.assign(sorted_monitors.begin(), sorted_monitors.begin() + n);

    return result;
}