# Component Architecture

```mermaid
classDiagram
class NoiseMonitor {
    -SoundSensor m_sound_sensor
    -DisplayManager m_display
    -DataLogger m_logger
    -SignalProcessor m_signal_processor
    -LedIndicator m_led_indicator
    -unsigned long m_last_display_time
    -unsigned long m_last_led_time
    +begin()
    +update()
    -handle_display()
}

class LedIndicator {
    -Adafruit_NeoPixel m_pixels
    -uint32_t[] LEVEL_COLORS
    +begin()
    +update()
    -update_level_display()
}

class SignalProcessor {
    -float m_ema_value
    -float m_baseline_ema
    -Statistics m_one_min_stats
    -Statistics m_fifteen_min_stats
    -Statistics m_daily_stats
    +process_sample()
    +get_noise_category()
    +get_current_value()
    +get_baseline()
    -update_statistics()
}

class DisplayManager {
    -U8G2 m_u8g2
    -int[] m_plot_buffer
    -int m_plot_index
    +begin()
    +update()
    +add_plot_point()
    -draw_stats()
    -draw_plot()
}

class DataLogger {
    -bool m_initialized
    +begin()
    +log_data()
    -create_headers()
}

class SoundSensor {
    +begin()
    +read_averaged_sample()
    -configure_adc()
}

NoiseMonitor --> SignalProcessor
NoiseMonitor --> DisplayManager
NoiseMonitor --> DataLogger
NoiseMonitor --> SoundSensor
NoiseMonitor --> LedIndicator
LedIndicator ..> SignalProcessor
DisplayManager ..> SignalProcessor
```
