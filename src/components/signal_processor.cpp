#include "signal_processor.hpp"

SignalProcessor::SignalProcessor() = default;

/*  *
 * @brief Process a single sample from the ADC.
 * @param raw_value The raw ADC value to process.
 */
void SignalProcessor::process_sample(uint16_t raw_value)
{
    // Update current value immediately
    m_ema_value = (config::signal_processing::EMA_ALPHA * raw_value) +
                  ((1.0f - config::signal_processing::EMA_ALPHA) * m_ema_value);

    // Update baseline more slowly
    if (m_baseline_ema == 0)
    {
        m_baseline_ema = m_ema_value;
    }
    m_baseline_ema = (config::signal_processing::BASELINE_ALPHA * m_ema_value) +
                     ((1.0f - config::signal_processing::BASELINE_ALPHA) * m_baseline_ema);

    // Update each statistic window independently
    unsigned long current_time = millis();

    // Update statistics for each time window
    update_statistics(m_one_min_stats, m_ema_value);
    update_statistics(m_fifteen_min_stats, m_ema_value);
    update_statistics(m_daily_stats, m_ema_value);
}

/**
 * @brief Update the EMA (Exponential Moving Average) value.
 * @param raw_value The raw ADC value to process.
 */
void SignalProcessor::update_ema(uint16_t raw_value)
{
    // Calculate fast EMA for current noise
    m_ema_value = (config::signal_processing::EMA_ALPHA * raw_value) +
                  ((1.0f - config::signal_processing::EMA_ALPHA) * m_ema_value);

    // Update baseline (very slow EMA)
    if (m_baseline_ema == 0)
    {
        m_baseline_ema = m_ema_value; // Initialize baseline
    }
    m_baseline_ema = (config::signal_processing::BASELINE_ALPHA * m_ema_value) +
                     ((1.0f - config::signal_processing::BASELINE_ALPHA) * m_baseline_ema);
}

/**
 * @brief Update the statistics for a given value.
 * @param stats The statistics structure to update.
 * @param value The value to update the statistics with.
 */
void SignalProcessor::update_statistics(Statistics &stats, float value)
{
    unsigned long current_time = millis();

    // Reset if window has expired
    if (current_time - stats.last_update > stats.window_size)
    {
        stats.min = stats.max = static_cast<uint16_t>(value);
        stats.avg = value;
        stats.samples = 1;
    }
    else
    {
        stats.min = std::min(stats.min, static_cast<uint16_t>(value));
        stats.max = std::max(stats.max, static_cast<uint16_t>(value));
        // Weighted average to prevent overflow
        stats.avg = (stats.avg * 0.95f) + (value * 0.05f);
        stats.samples++;
    }
    stats.last_update = current_time;
}

/**
 * @brief Get the noise category based on the EMA value.
 * @return The noise category.
 */
SignalProcessor::NoiseLevel SignalProcessor::get_noise_category() const
{
    // Use absolute values instead of ratios
    float current = m_ema_value;

    if (current < config::signal_processing::ranges::QUIET)
        return NoiseLevel::OK;
    if (current < config::signal_processing::ranges::MODERATE)
        return NoiseLevel::REGULAR;
    if (current < config::signal_processing::ranges::LOUD)
        return NoiseLevel::ELEVATED;
    return NoiseLevel::CRITICAL;
}