#pragma once
#include <string>
#include <vector>
#include <deque>
#include <memory>
#include "config/config.h"

class StatisticsMonitor
{
public:
    struct Config
    {
        std::string id;          // Unique identifier for API
        std::string label;       // Display label
        uint32_t period_ms;      // Monitoring period
        uint8_t priority;        // Display priority (lower = higher priority)
        size_t history_size{60}; // Number of historical points to keep
    };

    struct Stats
    {
        float current{0.0f};
        float min{INFINITY};
        float max{-INFINITY};
        float avg{0.0f};
        std::deque<float> history; // Rolling history for trends
        unsigned long last_update{0};
        uint32_t samples{0};
    };

    explicit StatisticsMonitor(const Config &config);

    // Core functionality
    void update(float value);
    void reset();

    // Getters
    const Stats &get_stats() const { return m_stats; }
    const Config &get_config() const { return m_config; }
    const std::string &get_id() const { return m_config.id; }
    uint8_t get_priority() const { return m_config.priority; }

private:
    Config m_config;
    Stats m_stats;

    void update_rolling_average(float value);
    void update_history(float value);
};