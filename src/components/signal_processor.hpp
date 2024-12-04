#pragma once

#include <Arduino.h>
#include "config/config.h"

/**
 * @brief Class representing the signal processor.
 */
class SignalProcessor
{
public:
    struct Statistics
    {
        uint16_t min = adc_config::MAX_VALUE;
        uint16_t max = 0;
        float avg = 0;
        uint32_t samples = 0;

        void reset()
        {
            min = adc_config::MAX_VALUE;
            max = 0;
            avg = 0;
            samples = 0;
        }
    };

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

    const Statistics &get_one_min_stats() const { return m_one_min_stats; }
    const Statistics &get_fifteen_min_stats() const { return m_fifteen_min_stats; }
    const Statistics &get_daily_stats() const { return m_daily_stats; }

private:
    float m_ema_value{0.0f};
    float m_baseline_ema{0.0f};

    Statistics m_one_min_stats;
    Statistics m_fifteen_min_stats;
    Statistics m_daily_stats;

    void update_ema(uint16_t raw_value);
    void update_statistics(Statistics &stats, float value);
};