#pragma once

#include <Arduino.h>
#include "config/config.h"
#include "config/configuration_manager.hpp"
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

    void begin();
    void begin(const JsonObject& config);

    bool update_config(const JsonObject& config);

    void process_sample(uint16_t raw_value);
    float get_current_value() const { return m_ema_value; }
    float get_baseline() const { return m_baseline_ema; }
    NoiseLevel get_noise_category() const;

    void add_monitor(const StatisticsMonitor::Config &config);
    void remove_monitor(const std::string &id);
    StatisticsMonitor *get_monitor(const std::string &id) const;
    std::vector<StatisticsMonitor *> get_priority_monitors(size_t count = 2) const;
    bool update_monitor_config(const JsonObject& config);

    float get_ema_alpha() const { return m_ema_alpha; }

private:
    std::vector<std::unique_ptr<StatisticsMonitor>> m_monitors;
    float m_ema_value{0.0f};
    float m_baseline_ema{0.0f};
    float m_ema_alpha{config::signal_processing::EMA_ALPHA};
    float m_baseline_alpha{config::signal_processing::BASELINE_ALPHA};

    float m_threshold_regular{config::signal_processing::thresholds::NOISE_REGULAR};
    float m_threshold_high{config::signal_processing::thresholds::NOISE_HIGH};
    float m_threshold_toxic{config::signal_processing::thresholds::NOISE_TOXIC};

    void update_ema(uint16_t raw_value);
    void update_monitors(float value);
    void load_monitor_configs();
    bool validate_config(const JsonObject& config);
    bool save_monitor_configs();
};