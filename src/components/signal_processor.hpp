#pragma once

#include <Arduino.h>
#include "config/config.h"

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

    struct Statistics
    {
        uint16_t min{UINT16_MAX};
        uint16_t max{0};
        float avg{0};
        uint32_t samples{0};
        unsigned long last_update{0};
        uint32_t window_size{0};

        Statistics(uint32_t window_ms = 60000) : window_size(window_ms) {}
    };

    SignalProcessor();

    void process_sample(uint16_t raw_value);
    float get_current_value() const { return m_ema_value; }
    float get_baseline() const { return m_baseline_ema; }
    NoiseLevel get_noise_category() const;

    const Statistics &get_one_min_stats() const { return m_one_min_stats; }
    const Statistics &get_fifteen_min_stats() const { return m_fifteen_min_stats; }
    const Statistics &get_daily_stats() const { return m_daily_stats; }

private:
    float m_ema_value{0.0f};
    float m_baseline_ema{0.0f};

    Statistics m_one_min_stats{60000};
    Statistics m_fifteen_min_stats{900000};
    Statistics m_daily_stats{86400000};

    void update_ema(uint16_t raw_value);
    void update_statistics(Statistics &stats, float value);
};