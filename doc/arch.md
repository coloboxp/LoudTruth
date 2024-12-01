```mermaid
classDiagram
class NoiseMonitor {
-SoundSensor sensor
-SignalProcessor processor
-DisplayManager display
-DataLogger logger
+begin()
+update()
}

    class SignalProcessor {
        -Statistics stats
        -float ema_value
        -float baseline_ema
        +process_sample()
        +get_noise_category()
    }

    class DisplayManager {
        -U8G2 display
        -int[] plot_buffer
        +update()
        -draw_stats()
        -draw_plot()
    }

    class DataLogger {
        -bool initialized
        +log_data()
        -create_headers()
    }

    NoiseMonitor --> SignalProcessor
    NoiseMonitor --> DisplayManager
    NoiseMonitor --> DataLogger
    DisplayManager ..> SignalProcessor
    DataLogger ..> SignalProcessor

```
