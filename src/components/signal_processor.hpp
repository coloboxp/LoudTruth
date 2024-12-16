#pragma once

#include <Arduino.h>
#include "config/config.h"
#include <vector>
#include <memory>
#include <algorithm>
#include "statistics_monitor.hpp"

/**
 * @brief Class representing the signal processor.
 */
class SignalProcessor
{
public:
    enum class NoiseLevel
    {
        OK,
        REGULAR,
        ELEVATED,
        CRITICAL
    };

    SignalProcessor();

    void process_sample(uint16_t raw_value);
    float get_current_value() const { return m_ema_value; }
    float get_baseline() const { return m_baseline_ema; }
    NoiseLevel get_noise_category() const;

    void add_monitor(const StatisticsMonitor::Config &config);
    void remove_monitor(const std::string &id);
    StatisticsMonitor *get_monitor(const std::string &id) const;
    std::vector<StatisticsMonitor *> get_priority_monitors(size_t count = 2) const;

    void begin();

private:
    static constexpr const char *CONFIG_FILE = "/monitors.json";
    void load_monitor_configs();
    void save_monitor_configs() const;
    void create_default_monitors();

    std::vector<std::unique_ptr<StatisticsMonitor>> m_monitors;
    float m_ema_value{0.0f};
    float m_baseline_ema{0.0f};

    void update_ema(uint16_t raw_value);
    void update_monitors(float value);
};