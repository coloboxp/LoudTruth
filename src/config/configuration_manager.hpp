#pragma once

#include <ArduinoJson.h>
#include <SD.h>
#include <memory>
#include "config.h"
#include "default_configs.h"

class ConfigurationManager {
public:
    static ConfigurationManager& instance() {
        static ConfigurationManager instance;
        return instance;
    }

    bool begin();
    
    // Configuration getters
    bool get_timing_config(JsonObject& config) const;
    bool get_signal_processing_config(JsonObject& config) const;
    bool get_hardware_config(JsonObject& config) const;
    bool get_display_config(JsonObject& config) const;
    bool get_alert_config(JsonObject& config) const;
    bool get_network_config(JsonObject& config) const;
    bool get_monitor_config(JsonObject& config) const;
    bool get_sound_sensor_config(JsonObject& config) const;
    bool get_led_config(JsonObject& config) const;
    bool get_logger_config(JsonObject& config) const;

    // Configuration setters
    bool update_timing_config(const JsonObject& config);
    bool update_signal_processing_config(const JsonObject& config);
    bool update_hardware_config(const JsonObject& config);
    bool update_display_config(const JsonObject& config);
    bool update_alert_config(const JsonObject& config);
    bool update_network_config(const JsonObject& config);
    bool update_monitor_config(const JsonObject& config);
    bool update_sound_sensor_config(const JsonObject& config);
    bool update_led_config(const JsonObject& config);
    bool update_logger_config(const JsonObject& config);

    // Utility methods
    bool reset_to_defaults();
    bool is_initialized() const { return m_initialized; }
    const char* get_last_error() const { return m_last_error.c_str(); }

    bool save_all_configs();
    bool save_monitor_configs();

private:
    ConfigurationManager() = default;
    ~ConfigurationManager() = default;
    ConfigurationManager(const ConfigurationManager&) = delete;
    ConfigurationManager& operator=(const ConfigurationManager&) = delete;

    static constexpr const char* CONFIG_DIR = "/config";
    static constexpr const char* TIMING_CONFIG_FILE = "/config/timing.json";
    static constexpr const char* SIGNAL_CONFIG_FILE = "/config/signal.json";
    static constexpr const char* HARDWARE_CONFIG_FILE = "/config/hardware.json";
    static constexpr const char* DISPLAY_CONFIG_FILE = "/config/display.json";
    static constexpr const char* ALERT_CONFIG_FILE = "/config/alert.json";
    static constexpr const char* NETWORK_CONFIG_FILE = "/config/network.json";
    static constexpr const char* MONITOR_CONFIG_FILE = "/config/monitor.json";
    static constexpr const char* SOUND_SENSOR_CONFIG_FILE = "/config/sound_sensor.json";
    static constexpr const char* LED_CONFIG_FILE = "/config/led.json";
    static constexpr const char* LOGGER_CONFIG_FILE = "/config/logger.json";

    bool m_initialized{false};
    std::string m_last_error;
    JsonDocument m_config_cache;

    // Internal helper methods
    bool init_sd_card();
    bool create_config_directory();
    bool load_all_configs();
    bool validate_config(const JsonObject& config, const char* config_type);
    void set_defaults();
    bool save_config_file(const char* filename, const JsonObject& config);
    bool load_config_file(const char* filename, JsonObject& config);
    bool write_default_config(const char* filename, const char* default_config);
}; 