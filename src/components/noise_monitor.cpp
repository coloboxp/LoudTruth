#include "noise_monitor.hpp"
#include <time.h>

NoiseMonitor::NoiseMonitor()
    : m_display_manager(m_alert_manager)
{
    // Empty constructor - initialization moved to begin()
}

bool NoiseMonitor::load_configuration()
{
    JsonDocument doc;
    
    // Load timing configuration
    JsonObject timing_config = doc.to<JsonObject>();
    if (!ConfigurationManager::instance().get_timing_config(timing_config)) {
        Serial.println("Failed to load timing configuration, using defaults");
        m_sample_interval = config::timing::TimingConfig::DEFAULT_SAMPLE_INTERVAL;
        m_display_interval = config::timing::TimingConfig::DEFAULT_DISPLAY_INTERVAL;
        m_logging_interval = config::timing::TimingConfig::DEFAULT_LOGGING_INTERVAL;
        m_led_update_interval = config::timing::TimingConfig::DEFAULT_LED_UPDATE_INTERVAL;
    } else {
        m_sample_interval = timing_config["sample_interval"] | config::timing::TimingConfig::DEFAULT_SAMPLE_INTERVAL;
        m_display_interval = timing_config["display_interval"] | config::timing::TimingConfig::DEFAULT_DISPLAY_INTERVAL;
        m_logging_interval = timing_config["logging_interval"] | config::timing::TimingConfig::DEFAULT_LOGGING_INTERVAL;
        m_led_update_interval = timing_config["led_update_interval"] | config::timing::TimingConfig::DEFAULT_LED_UPDATE_INTERVAL;
    }

    // Load signal processing configuration
    JsonObject signal_config = doc.to<JsonObject>();
    if (ConfigurationManager::instance().get_signal_processing_config(signal_config)) {
        m_signal_processor.begin(signal_config);
    } else {
        Serial.println("Failed to load signal processing configuration, using defaults");
        m_signal_processor.begin();
    }

    return true;
}

void NoiseMonitor::apply_configuration()
{
    // Apply configurations to components that support runtime updates
    JsonDocument doc;
    
    JsonObject display_config = doc.as<JsonObject>();
    if (ConfigurationManager::instance().get_display_config(display_config)) {
        m_display_manager.update_config(display_config);
    }

    JsonObject alert_config = doc.as<JsonObject>();
    if (ConfigurationManager::instance().get_alert_config(alert_config)) {
        m_alert_manager.update_config(alert_config);
    }

    JsonObject signal_config = doc.as<JsonObject>();
    if (ConfigurationManager::instance().get_signal_processing_config(signal_config)) {
        m_signal_processor.update_config(signal_config);
    }
}

bool NoiseMonitor::begin()
{
    // Initialize configuration manager first
    if (!ConfigurationManager::instance().begin()) {
        Serial.println("Warning: Configuration manager initialization failed, using defaults");
    }

    // Load and apply configurations
    load_configuration();

    // Initialize components one at a time with delays
    m_sound_sensor.begin();
    delay(100); // Give ADC time to stabilize

    m_display_manager.begin();
    delay(100); // Give display time to initialize

    m_led_indicator.begin();
    delay(50); // Brief delay for LED strip

    m_alert_manager.begin();
    delay(50); // Brief delay for alert system

    bool logger_ok = m_logger.begin();
    delay(50); // Give SD card time to initialize

    // Initialize API server with required components
    m_api_server.set_signal_processor(&m_signal_processor);
    m_api_server.set_display_manager(&m_display_manager);
    m_api_server.begin();

    // Apply runtime configurations after components are initialized
    apply_configuration();

    return logger_ok;
}

void NoiseMonitor::update()
{
    unsigned long current_time = millis();

    // Handle sampling based on configured interval
    if (current_time - m_last_sample_time >= m_sample_interval) {
        handle_sampling();
        m_last_sample_time = current_time;
    }

    // Handle display updates based on configured interval
    if (current_time - m_last_display_time >= m_display_interval) {
        handle_display();
        m_last_display_time = current_time;
    }

    // Handle LED updates based on configured interval
    if (current_time - m_last_led_time >= m_led_update_interval) {
        m_led_indicator.update(m_signal_processor);
        m_last_led_time = current_time;
    }

    // Handle logging based on configured interval
    if (current_time - m_last_log_time >= m_logging_interval) {
        handle_logging();
        m_last_log_time = current_time;
    }

    // Handle API server updates
    handle_api_update();

    // Update alert manager
    m_alert_manager.update(m_signal_processor);
}

/**
 * @brief Handle the sampling task.
 */
void NoiseMonitor::handle_sampling()
{
    uint32_t current_time = millis();
    
    if (current_time - m_last_sample_time >= config::timing::SAMPLE_INTERVAL) {
        if (m_sound_sensor.is_ready()) {
            uint16_t raw_value = m_sound_sensor.read();
            m_signal_processor.process_sample(raw_value);
            m_last_sample_time = current_time;
        }
    }
}

/**
 * @brief Handle the display task.
 */
void NoiseMonitor::handle_display()
{
    unsigned long current_time = millis();

    // Update display at slower rate
    if (current_time - m_last_display_time >= config::timing::DISPLAY_INTERVAL)
    {
        m_display_manager.update(m_signal_processor);
        m_last_display_time = current_time;
    }

    // Update LED indicator at faster rate
    if (current_time - m_last_led_time >= config::timing::LED_UPDATE_INTERVAL)
    {
        m_led_indicator.update(m_signal_processor);
        m_last_led_time = current_time;
    }
}

/**
 * @brief Handle the logging task.
 */
void NoiseMonitor::handle_logging()
{
    unsigned long current_time = millis();

    if (current_time - m_last_log_time >= config::timing::LOGGING_INTERVAL)
    {
        m_logger.log_data(m_signal_processor);
        m_last_log_time = current_time;
    }
}

void NoiseMonitor::handle_api_update()
{
    unsigned long current_time = millis();

    // Only attempt API updates if enough time has passed
    if (current_time - m_last_api_time >= config::thingspeak::UPDATE_INTERVAL_MS)
    {
        // Create a temporary JsonDocument
        JsonDocument doc;

        // Get current timestamp
        struct tm timeinfo;
        if (getLocalTime(&timeinfo))
        {
            char timestamp[30];
            strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);

            // Add data to JSON document
            doc["created_at"] = timestamp;
            doc["field1"] = m_signal_processor.get_current_value();
            doc["field2"] = m_signal_processor.get_baseline();
            doc["field3"] = static_cast<int>(m_signal_processor.get_noise_category());

            // Send data to ThingSpeak
            if (ApiHandler::instance().send_noise_data(doc))
            {
                ESP_LOGI("NoiseMonitor", "Data sent to ThingSpeak at %s", timestamp);
            }
        }
        m_last_api_time = current_time;
    }
}