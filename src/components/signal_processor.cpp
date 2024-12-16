#include "signal_processor.hpp"
#include <ArduinoJson.h>
#include <SD.h>

SignalProcessor::SignalProcessor()
    : m_ema_value(0.0f), m_baseline_ema(0.0f)
{
    // Constructor only initializes members
}

void SignalProcessor::begin()
{
    load_monitor_configs();
    // If no monitors were loaded, create defaults
    if (m_monitors.empty())
    {
        create_default_monitors();
    }
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

void SignalProcessor::load_monitor_configs()
{
    if (!SD.exists(CONFIG_FILE))
    {
        return; // No config file exists yet
    }

    File file = SD.open(CONFIG_FILE, FILE_READ);
    if (!file)
    {
        return; // Failed to open file
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        return; // Failed to parse JSON
    }

    JsonArray monitors = doc["monitors"].to<JsonArray>();
    for (JsonObject monitor_json : monitors)
    {
        StatisticsMonitor::Config config;
        config.id = monitor_json["id"].as<const char *>();
        config.label = monitor_json["label"].as<const char *>();
        config.period_ms = monitor_json["period_ms"];
        config.priority = monitor_json["priority"];

        // Use emplace_back with direct construction instead of make_unique
        m_monitors.emplace_back(new StatisticsMonitor(config));
    }
}

void SignalProcessor::save_monitor_configs() const
{
    JsonDocument doc;
    auto monitors = doc["monitors"].to<JsonArray>();

    for (const auto &monitor : m_monitors)
    {
        auto monitor_obj = monitors.add<JsonObject>();
        const auto &config = monitor->get_config();
        monitor_obj["id"] = config.id;
        monitor_obj["label"] = config.label;
        monitor_obj["period_ms"] = config.period_ms;
        monitor_obj["priority"] = config.priority;
    }

    File file = SD.open(CONFIG_FILE, FILE_WRITE);
    if (!file)
    {
        return; // Failed to open file
    }

    serializeJson(doc, file);
    file.close();
}

void SignalProcessor::create_default_monitors()
{
    // Create 1-minute monitor
    StatisticsMonitor::Config one_min;
    one_min.id = "1min";
    one_min.label = "1 Min";
    one_min.period_ms = 60000; // 1 minute
    one_min.priority = 1;
    one_min.history_size = 60;
    add_monitor(one_min);

    // Create 15-minute monitor
    StatisticsMonitor::Config fifteen_min;
    fifteen_min.id = "15min";
    fifteen_min.label = "15 Min";
    fifteen_min.period_ms = 900000; // 15 minutes
    fifteen_min.priority = 2;
    fifteen_min.history_size = 60;
    add_monitor(fifteen_min);

    // Save the default configuration
    save_monitor_configs();
}

void SignalProcessor::add_monitor(const StatisticsMonitor::Config &config)
{
    // Use emplace_back with direct construction
    m_monitors.emplace_back(new StatisticsMonitor(config));
    save_monitor_configs();
}

void SignalProcessor::remove_monitor(const std::string &id)
{
    m_monitors.erase(
        std::remove_if(m_monitors.begin(), m_monitors.end(),
                       [&id](const std::unique_ptr<StatisticsMonitor> &monitor)
                       {
                           return monitor->get_id() == id;
                       }),
        m_monitors.end());
    save_monitor_configs();
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
    result.reserve(count);

    // Create a temporary vector of pointers for sorting
    std::vector<StatisticsMonitor *> sorted_monitors;
    sorted_monitors.reserve(m_monitors.size());

    for (const auto &monitor : m_monitors)
    {
        sorted_monitors.push_back(monitor.get());
    }

    // Sort by priority using explicit types
    std::sort(sorted_monitors.begin(), sorted_monitors.end(),
              [](StatisticsMonitor *a, StatisticsMonitor *b)
              {
                  return a->get_config().priority < b->get_config().priority;
              });

    // Take the requested number of monitors
    for (size_t i = 0; i < std::min(count, sorted_monitors.size()); ++i)
    {
        result.push_back(sorted_monitors[i]);
    }

    return result;
}