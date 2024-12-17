#pragma once

namespace config {
namespace defaults {

// Default timing configuration
constexpr char TIMING_CONFIG[] = R"({
    "sample_interval": 10,
    "display_interval": 100,
    "logging_interval": 1000,
    "led_update_interval": 50
})";

// Default signal processing configuration
constexpr char SIGNAL_PROCESSING_CONFIG[] = R"({
    "ema_alpha": 0.1,
    "baseline_alpha": 0.001,
    "thresholds": {
        "regular": 1.2,
        "high": 1.5,
        "toxic": 2.0
    },
    "ranges": {
        "min": 0.0,
        "max": 100.0
    }
})";

// Default display configuration
constexpr char DISPLAY_CONFIG[] = R"({
    "plot_points": 128,
    "plot_height": 40,
    "plot_baseline_y": 48,
    "contrast": 128,
    "backlight_timeout_ms": 5000,
    "auto_backlight": true,
    "flip_display": false
})";

// Default LED configuration
constexpr char LED_CONFIG[] = R"({
    "brightness": 50,
    "reverse_direction": false,
    "animation_mode": 0,
    "animation_speed": 1000,
    "colors": [
        {"r": 0, "g": 255, "b": 0},
        {"r": 0, "g": 255, "b": 0},
        {"r": 255, "g": 255, "b": 0},
        {"r": 255, "g": 255, "b": 0},
        {"r": 255, "g": 128, "b": 0},
        {"r": 255, "g": 0, "b": 0},
        {"r": 255, "g": 0, "b": 0},
        {"r": 255, "g": 0, "b": 0}
    ]
})";

// Default alert configuration
constexpr char ALERT_CONFIG[] = R"({
    "elevated_threshold_ms": 5000,
    "beep_duration_ms": 100,
    "beep_interval_ms": 500,
    "max_alerts": 3,
    "base_cooldown_ms": 30000,
    "max_cooldown_ms": 300000,
    "rapid_trigger_window_ms": 60000,
    "alarm_frequency": 2000,
    "alarm_frequency_2": 2500
})";

// Default logger configuration
constexpr char LOGGER_CONFIG[] = R"({
    "enabled": true,
    "directory": "/logs",
    "max_file_size": 1048576,
    "rotation_interval": 86400,
    "max_files": 7,
    "include_timestamps": true,
    "include_baseline": true,
    "include_categories": true,
    "decimal_precision": 2
})";

// Default sound sensor configuration
constexpr char SOUND_SENSOR_CONFIG[] = R"({
    "adc_width": 12,
    "adc_attenuation": 11,
    "sampling_rate": 1000,
    "averaging_samples": 4,
    "voltage_reference": 3.3,
    "calibration_offset": 0.0,
    "calibration_scale": 1.0
})";

// Default monitor configuration
constexpr char MONITOR_CONFIG[] = R"({
    "monitors": [
        {
            "id": "1min",
            "label": "1 Minute",
            "period_ms": 60000,
            "history_size": 60,
            "priority": 1
        },
        {
            "id": "5min",
            "label": "5 Minutes",
            "period_ms": 300000,
            "history_size": 60,
            "priority": 2
        }
    ]
})";

} // namespace defaults
} // namespace config 