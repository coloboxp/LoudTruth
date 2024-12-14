# Data Flow

```mermaid
sequenceDiagram
participant Main
participant NoiseMonitor
participant SoundSensor
participant SignalProcessor
participant DisplayManager
participant DataLogger
participant LedIndicator
participant WiFiManager
participant ApiHandler

    Main->>NoiseMonitor: begin()
    NoiseMonitor->>SoundSensor: begin()
    NoiseMonitor->>DisplayManager: begin()
    NoiseMonitor->>DataLogger: begin()
    NoiseMonitor->>LedIndicator: begin()

    loop Every SAMPLE_INTERVAL (10ms)
        NoiseMonitor->>SoundSensor: read_averaged_sample()
        SoundSensor-->>NoiseMonitor: raw_value
        NoiseMonitor->>SignalProcessor: process_sample(raw_value)
        SignalProcessor-->>SignalProcessor: update_statistics()
    end

    loop Every LED_UPDATE_INTERVAL (50ms)
        NoiseMonitor->>LedIndicator: update(signal_processor)
        LedIndicator->>LedIndicator: update_level_display()
    end

    loop Every DISPLAY_INTERVAL (500ms)
        NoiseMonitor->>DisplayManager: update(signal_processor)
        DisplayManager->>WiFiManager: is_connected()
        DisplayManager->>ApiHandler: m_available
        DisplayManager->>DisplayManager: draw_stats()
        DisplayManager->>DisplayManager: draw_plot()
    end

    loop Every LOG_INTERVAL (60s)
        NoiseMonitor->>DataLogger: log_data(signal_processor)
        DataLogger->>DataLogger: write to SD
    end
```
