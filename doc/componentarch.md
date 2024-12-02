```mermaid
classDiagram
class NoiseMonitor {
-SoundSensor m_sound_sensor
-DisplayManager m_display
-DataLogger m_logger
-SignalProcessor m_signal_processor
+begin()
+update()
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
    }

    class DisplayManager {
        -U8G2 m_u8g2
        -int[] m_plot_buffer
        -int m_plot_index
        +begin()
        +update()
        +add_plot_point()
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
