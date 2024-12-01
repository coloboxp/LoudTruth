#include "signal_processor.hpp"

SignalProcessor::SignalProcessor() = default;

void SignalProcessor::process_sample(uint16_t raw_value)
{
    update_ema(raw_value);
    update_statistics(m_one_min_stats, m_ema_value);
    update_statistics(m_fifteen_min_stats, m_ema_value);
    update_statistics(m_daily_stats, m_ema_value);
}

void SignalProcessor::update_ema(uint16_t raw_value)
{
    // Calculate fast EMA for current noise
    m_ema_value = (signal_processing::EMA_ALPHA * raw_value) +
                  ((1.0f - signal_processing::EMA_ALPHA) * m_ema_value);

    // Update baseline (very slow EMA)
    if (m_baseline_ema == 0)
    {
        m_baseline_ema = m_ema_value; // Initialize baseline
    }
    m_baseline_ema = (signal_processing::BASELINE_ALPHA * m_ema_value) +
                     ((1.0f - signal_processing::BASELINE_ALPHA) * m_baseline_ema);
}

void SignalProcessor::update_statistics(Statistics &stats, float value)
{
    stats.min = min(stats.min, (int)value);
    stats.max = max(stats.max, (int)value);
    stats.avg = ((stats.avg * stats.samples) + value) / (stats.samples + 1);
    stats.samples++;
}

SignalProcessor::NoiseLevel SignalProcessor::get_noise_category() const
{
    float ratio = m_ema_value / m_baseline_ema;

    if (ratio < signal_processing::thresholds::NOISE_REGULAR)
        return NoiseLevel::OK;
    if (ratio < signal_processing::thresholds::NOISE_HIGH)
        return NoiseLevel::REGULAR;
    if (ratio < signal_processing::thresholds::NOISE_TOXIC)
        return NoiseLevel::ELEVATED;
    return NoiseLevel::CRITICAL;
}