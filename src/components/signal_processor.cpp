#include "signal_processor.hpp"
#include <esp_log.h>

static const char* TAG = "SignalProcessor";

SignalProcessor::SignalProcessor() = default;

void SignalProcessor::begin() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_signal_processing_config(config)) {
        begin(config);
    } else {
        ESP_LOGW(TAG, "Failed to load signal processing config, using defaults");
        // Use default values from config.h
        m_ema_alpha = config::signal_processing::EMA_ALPHA;
        m_baseline_alpha = config::signal_processing::BASELINE_ALPHA;
        m_threshold_regular = config::signal_processing::thresholds::NOISE_REGULAR;
        m_threshold_high = config::signal_processing::thresholds::NOISE_HIGH;
        m_threshold_toxic = config::signal_processing::thresholds::NOISE_TOXIC;
    }

    // Load monitor configurations
    load_monitor_configs();
}

void SignalProcessor::begin(const JsonObject& config) {
    if (validate_config(config)) {
        m_ema_alpha = config["ema_alpha"] | config::signal_processing::EMA_ALPHA;
        m_baseline_alpha = config["baseline_alpha"] | config::signal_processing::BASELINE_ALPHA;
        
        if (config["thresholds"].is<JsonObject>()) {
            const JsonObject& thresholds = config["thresholds"];
            m_threshold_regular = thresholds["regular"] | config::signal_processing::thresholds::NOISE_REGULAR;
            m_threshold_high = thresholds["high"] | config::signal_processing::thresholds::NOISE_HIGH;
            m_threshold_toxic = thresholds["toxic"] | config::signal_processing::thresholds::NOISE_TOXIC;
        }
    } else {
        ESP_LOGW(TAG, "Invalid signal processing config, using defaults");
    }

    // Load monitor configurations
    load_monitor_configs();
}

bool SignalProcessor::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    // Update configuration values
    if (config["ema_alpha"].is<float>()) {
        m_ema_alpha = config["ema_alpha"].as<float>();
    }
    
    if (config["baseline_alpha"].is<float>()) {
        m_baseline_alpha = config["baseline_alpha"].as<float>();
    }

    if (config["thresholds"].is<JsonObject>()) {
        const JsonObject& thresholds = config["thresholds"];
        if (thresholds["regular"].is<float>()) {
            m_threshold_regular = thresholds["regular"].as<float>();
        }
        if (thresholds["high"].is<float>()) {
            m_threshold_high = thresholds["high"].as<float>();
        }
        if (thresholds["toxic"].is<float>()) {
            m_threshold_toxic = thresholds["toxic"].as<float>();
        }
    }

    return true;
}

void SignalProcessor::process_sample(uint16_t raw_value) {
    update_ema(raw_value);
    update_monitors(m_ema_value);
}

void SignalProcessor::update_ema(uint16_t raw_value) {
    // Update EMA for current value
    m_ema_value = (m_ema_alpha * raw_value) + ((1.0f - m_ema_alpha) * m_ema_value);
    
    // Update baseline with slower alpha
    m_baseline_ema = (m_baseline_alpha * raw_value) + ((1.0f - m_baseline_alpha) * m_baseline_ema);
}

SignalProcessor::NoiseLevel SignalProcessor::get_noise_category() const {
    float ratio = m_ema_value / m_baseline_ema;

    if (ratio < m_threshold_regular)
        return NoiseLevel::OK;
    if (ratio < m_threshold_high)
        return NoiseLevel::REGULAR;
    if (ratio < m_threshold_toxic)
        return NoiseLevel::ELEVATED;
    return NoiseLevel::CRITICAL;
}

void SignalProcessor::load_monitor_configs() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (!ConfigurationManager::instance().get_monitor_config(config)) {
        ESP_LOGW(TAG, "Failed to load monitor configurations");
        return;
    }

    m_monitors.clear();

    JsonArray monitors = config["monitors"].as<JsonArray>();
    for (JsonObject monitor_config : monitors) {
        StatisticsMonitor::Config monitor_cfg;
        monitor_cfg.id = monitor_config["id"].as<const char*>();
        monitor_cfg.label = monitor_config["label"].as<const char*>();
        monitor_cfg.period_ms = monitor_config["period_ms"] | 60000;
        monitor_cfg.priority = monitor_config["priority"] | 99;
        monitor_cfg.history_size = monitor_config["history_size"] | 60;

        add_monitor(monitor_cfg);
    }
}

bool SignalProcessor::update_monitor_config(const JsonObject& config) {
    if (!config["monitors"].is<JsonArray>()) {
        return false;
    }

    m_monitors.clear();
    JsonArray monitors = config["monitors"].as<JsonArray>();
    
    for (JsonObject monitor_config : monitors) {
        StatisticsMonitor::Config monitor_cfg;
        monitor_cfg.id = monitor_config["id"].as<const char*>();
        monitor_cfg.label = monitor_config["label"].as<const char*>();
        monitor_cfg.period_ms = monitor_config["period_ms"] | 60000;
        monitor_cfg.priority = monitor_config["priority"] | 99;
        monitor_cfg.history_size = monitor_config["history_size"] | 60;

        add_monitor(monitor_cfg);
    }

    return true;
}

bool SignalProcessor::validate_config(const JsonObject& config) {
    if (config["ema_alpha"].is<float>()) {
        float alpha = config["ema_alpha"].as<float>();
        if (alpha <= 0.0f || alpha > 1.0f) {
            return false;
        }
    }

    if (config["baseline_alpha"].is<float>()) {
        float alpha = config["baseline_alpha"].as<float>();
        if (alpha <= 0.0f || alpha > 1.0f) {
            return false;
        }
    }

    if (config["thresholds"].is<JsonObject>()) {
        const JsonObject& thresholds = config["thresholds"];
        if (thresholds["regular"].is<float>() &&
            thresholds["high"].is<float>() &&
            thresholds["toxic"].is<float>()) {
            float regular = thresholds["regular"].as<float>();
            float high = thresholds["high"].as<float>();
            float toxic = thresholds["toxic"].as<float>();
            
            if (regular >= high || high >= toxic) {
                return false;
            }
        }
    }

    return true;
}

void SignalProcessor::update_monitors(float value)
{
    for (auto &monitor : m_monitors)
    {
        monitor->update(value);
    }
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

bool SignalProcessor::save_monitor_configs() {
    JsonDocument doc;
    JsonArray monitors = doc.to<JsonArray>();

    for (const auto& monitor : m_monitors) {
        JsonObject mon = monitors.add<JsonObject>();
        mon["id"] = monitor->get_id();
        mon["label"] = monitor->get_config().label;
        mon["period_ms"] = monitor->get_config().period_ms;
        mon["priority"] = monitor->get_config().priority;
        mon["history_size"] = monitor->get_config().history_size;
    }

    return ConfigurationManager::instance().update_monitor_config(doc.as<JsonObject>());
}