#include "configuration_manager.hpp"
#include <esp_log.h>

static const char* TAG = "ConfigManager";

bool ConfigurationManager::begin() {
    if (!init_sd_card()) {
        m_last_error = "Failed to initialize SD card";
        ESP_LOGE(TAG, "%s", m_last_error.c_str());
        return false;
    }

    if (!create_config_directory()) {
        m_last_error = "Failed to create config directory";
        ESP_LOGE(TAG, "%s", m_last_error.c_str());
        return false;
    }

    if (!load_all_configs()) {
        m_last_error = "Failed to load configurations";
        ESP_LOGE(TAG, "%s", m_last_error.c_str());
        set_defaults();
        if (!save_all_configs()) {
            m_last_error = "Failed to save default configurations";
            ESP_LOGE(TAG, "%s", m_last_error.c_str());
            return false;
        }
    }

    m_initialized = true;
    return true;
}

bool ConfigurationManager::init_sd_card() {
    return SD.begin(config::hardware::pins::sd::CS);
}

bool ConfigurationManager::create_config_directory() {
    if (!SD.exists(CONFIG_DIR)) {
        return SD.mkdir(CONFIG_DIR);
    }
    return true;
}

bool ConfigurationManager::load_all_configs() {
    bool success = true;
    
    // Create JsonObjects using as<JsonObject>()
    JsonObject timing = m_config_cache["timing"].as<JsonObject>();
    JsonObject signal = m_config_cache["signal_processing"].as<JsonObject>();
    JsonObject display = m_config_cache["display"].as<JsonObject>();
    JsonObject alert = m_config_cache["alert"].as<JsonObject>();
    JsonObject led = m_config_cache["led"].as<JsonObject>();
    JsonObject sound = m_config_cache["sound_sensor"].as<JsonObject>();
    JsonObject logger = m_config_cache["logger"].as<JsonObject>();
    JsonObject monitor = m_config_cache["monitor"].as<JsonObject>();

    // Load configurations using the objects
    success &= load_config_file(TIMING_CONFIG_FILE, timing) ||
               write_default_config(TIMING_CONFIG_FILE, config::defaults::TIMING_CONFIG);
    success &= load_config_file(SIGNAL_CONFIG_FILE, signal) ||
               write_default_config(SIGNAL_CONFIG_FILE, config::defaults::SIGNAL_PROCESSING_CONFIG);
    success &= load_config_file(DISPLAY_CONFIG_FILE, display) ||
               write_default_config(DISPLAY_CONFIG_FILE, config::defaults::DISPLAY_CONFIG);
    success &= load_config_file(ALERT_CONFIG_FILE, alert) ||
               write_default_config(ALERT_CONFIG_FILE, config::defaults::ALERT_CONFIG);
    success &= load_config_file(LED_CONFIG_FILE, led) ||
               write_default_config(LED_CONFIG_FILE, config::defaults::LED_CONFIG);
    success &= load_config_file(SOUND_SENSOR_CONFIG_FILE, sound) ||
               write_default_config(SOUND_SENSOR_CONFIG_FILE, config::defaults::SOUND_SENSOR_CONFIG);
    success &= load_config_file(LOGGER_CONFIG_FILE, logger) ||
               write_default_config(LOGGER_CONFIG_FILE, config::defaults::LOGGER_CONFIG);
    success &= load_config_file(MONITOR_CONFIG_FILE, monitor) ||
               write_default_config(MONITOR_CONFIG_FILE, config::defaults::MONITOR_CONFIG);

    return success;
}

bool ConfigurationManager::write_default_config(const char* filename, const char* default_config) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        ESP_LOGE(TAG, "Failed to create config file: %s", filename);
        return false;
    }

    if (file.print(default_config) != strlen(default_config)) {
        ESP_LOGE(TAG, "Failed to write default config to: %s", filename);
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool ConfigurationManager::load_config_file(const char* filename, JsonObject& config) {
    if (!SD.exists(filename)) {
        return false;
    }

    File file = SD.open(filename);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open config file: %s", filename);
        return false;
    }

    // Create temporary document for file parsing
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error) {
        ESP_LOGE(TAG, "Failed to parse config file: %s, error: %s", filename, error.c_str());
        return false;
    }

    // Copy the parsed document to the provided config object
    config.set(doc.as<JsonObject>());
    return true;
}

bool ConfigurationManager::save_config_file(const char* filename, const JsonObject& config) {
    File file = SD.open(filename, FILE_WRITE);
    if (!file) {
        ESP_LOGE(TAG, "Failed to open config file for writing: %s", filename);
        return false;
    }

    if (serializeJson(config, file) == 0) {
        ESP_LOGE(TAG, "Failed to write config to file: %s", filename);
        file.close();
        return false;
    }

    file.close();
    return true;
}

// Implementation of getter methods
bool ConfigurationManager::get_timing_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["timing"]);
    return true;
}

bool ConfigurationManager::get_signal_processing_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["signal_processing"]);
    return true;
}

bool ConfigurationManager::get_hardware_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["hardware"]);
    return true;
}

bool ConfigurationManager::get_display_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["display"]);
    return true;
}

bool ConfigurationManager::get_alert_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["alert"]);
    return true;
}

bool ConfigurationManager::get_network_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["network"]);
    return true;
}

bool ConfigurationManager::get_monitor_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["monitor"]);
    return true;
}

bool ConfigurationManager::get_sound_sensor_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["sound_sensor"]);
    return true;
}

bool ConfigurationManager::get_led_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["led"]);
    return true;
}

bool ConfigurationManager::get_logger_config(JsonObject& config) const {
    if (!m_initialized) return false;
    config.set(m_config_cache["logger"]);
    return true;
}

// Implementation of update methods
bool ConfigurationManager::update_timing_config(const JsonObject& config) {
    if (!validate_config(config, "timing")) return false;
    m_config_cache["timing"].set(config);
    return save_config_file(TIMING_CONFIG_FILE, m_config_cache["timing"].as<JsonObject>());
}

bool ConfigurationManager::update_signal_processing_config(const JsonObject& config) {
    if (!validate_config(config, "signal_processing")) return false;
    m_config_cache["signal_processing"].set(config);
    return save_config_file(SIGNAL_CONFIG_FILE, config);
}

bool ConfigurationManager::update_hardware_config(const JsonObject& config) {
    if (!validate_config(config, "hardware")) return false;
    m_config_cache["hardware"].set(config);
    return save_config_file(HARDWARE_CONFIG_FILE, config);
}

bool ConfigurationManager::update_display_config(const JsonObject& config) {
    if (!validate_config(config, "display")) return false;
    m_config_cache["display"].set(config);
    return save_config_file(DISPLAY_CONFIG_FILE, config);
}

bool ConfigurationManager::update_alert_config(const JsonObject& config) {
    if (!validate_config(config, "alert")) return false;
    m_config_cache["alert"].set(config);
    return save_config_file(ALERT_CONFIG_FILE, config);
}

bool ConfigurationManager::update_network_config(const JsonObject& config) {
    if (!validate_config(config, "network")) return false;
    m_config_cache["network"].set(config);
    return save_config_file(NETWORK_CONFIG_FILE, config);
}

bool ConfigurationManager::update_monitor_config(const JsonObject& config) {
    if (!validate_config(config, "monitor")) return false;
    m_config_cache["monitor"].set(config);
    return save_config_file(MONITOR_CONFIG_FILE, config);
}

bool ConfigurationManager::update_sound_sensor_config(const JsonObject& config) {
    if (!validate_config(config, "sound_sensor")) return false;
    m_config_cache["sound_sensor"].set(config);
    return save_config_file(SOUND_SENSOR_CONFIG_FILE, config);
}

bool ConfigurationManager::update_led_config(const JsonObject& config) {
    if (!validate_config(config, "led")) return false;
    m_config_cache["led"].set(config);
    return save_config_file(LED_CONFIG_FILE, config);
}

bool ConfigurationManager::update_logger_config(const JsonObject& config) {
    if (!validate_config(config, "logger")) return false;
    m_config_cache["logger"].set(config);
    return save_config_file(LOGGER_CONFIG_FILE, config);
}

bool ConfigurationManager::validate_config(const JsonObject& config, const char* config_type) {
    if (!m_initialized) {
        m_last_error = "Configuration manager not initialized";
        return false;
    }

    if (config.isNull()) {
        m_last_error = "Invalid configuration object";
        return false;
    }

    // Specific validation for each config type
    if (strcmp(config_type, "timing") == 0) {
        if (!config["sample_interval"].is<uint32_t>() ||
            !config["display_interval"].is<uint32_t>() ||
            !config["logging_interval"].is<uint32_t>() ||
            !config["led_update_interval"].is<uint32_t>()) {
            m_last_error = "Invalid timing configuration format";
            return false;
        }
    }
    else if (strcmp(config_type, "signal_processing") == 0) {
        if (!config["ema_alpha"].is<float>() ||
            !config["baseline_alpha"].is<float>() ||
            !config["thresholds"].is<JsonObject>()) {
            m_last_error = "Invalid signal processing configuration format";
            return false;
        }
        
        // Additional validation from SignalProcessor::validate_config
        float ema_alpha = config["ema_alpha"].as<float>();
        float baseline_alpha = config["baseline_alpha"].as<float>();
        if (ema_alpha <= 0.0f || ema_alpha > 1.0f ||
            baseline_alpha <= 0.0f || baseline_alpha > 1.0f) {
            m_last_error = "Invalid alpha values in signal processing config";
            return false;
        }
    }
    else if (strcmp(config_type, "display") == 0) {
        if (!config["plot_points"].is<uint8_t>() ||
            !config["plot_height"].is<uint8_t>() ||
            !config["plot_baseline_y"].is<uint8_t>() ||
            !config["contrast"].is<uint8_t>() ||
            !config["backlight_timeout_ms"].is<uint16_t>()) {
            m_last_error = "Invalid display configuration format";
            return false;
        }
    }
    else if (strcmp(config_type, "alert") == 0) {
        if (!config["elevated_threshold_ms"].is<uint32_t>() ||
            !config["beep_duration_ms"].is<uint32_t>() ||
            !config["beep_interval_ms"].is<uint32_t>() ||
            !config["max_alerts"].is<uint8_t>()) {
            m_last_error = "Invalid alert configuration format";
            return false;
        }
    }
    else if (strcmp(config_type, "led") == 0) {
        if (!config["brightness"].is<uint8_t>() ||
            !config["colors"].is<JsonArray>()) {
            m_last_error = "Invalid LED configuration format";
            return false;
        }
        
        uint8_t brightness = config["brightness"].as<uint8_t>();
        if (brightness > 255) {
            m_last_error = "Invalid LED brightness value";
            return false;
        }
    }
    else if (strcmp(config_type, "sound_sensor") == 0) {
        if (!config["adc_width"].is<uint8_t>() ||
            !config["adc_attenuation"].is<uint8_t>() ||
            !config["sampling_rate"].is<uint16_t>() ||
            !config["averaging_samples"].is<uint8_t>()) {
            m_last_error = "Invalid sound sensor configuration format";
            return false;
        }
        
        uint16_t sampling_rate = config["sampling_rate"].as<uint16_t>();
        uint8_t averaging_samples = config["averaging_samples"].as<uint8_t>();
        if (sampling_rate < 100 || sampling_rate > 10000 ||
            averaging_samples < 1 || averaging_samples > 64) {
            m_last_error = "Invalid sound sensor sampling parameters";
            return false;
        }
    }
    else if (strcmp(config_type, "logger") == 0) {
        if (!config["enabled"].is<bool>() ||
            !config["max_file_size"].is<uint32_t>() ||
            !config["max_files"].is<uint8_t>()) {
            m_last_error = "Invalid logger configuration format";
            return false;
        }
        
        // Additional validation from DataLogger::validate_config
        uint32_t max_file_size = config["max_file_size"].as<uint32_t>();
        uint8_t max_files = config["max_files"].as<uint8_t>();
        if (max_file_size < 1024 || max_file_size > (10 * 1024 * 1024) ||
            max_files < 1 || max_files > 31) {
            m_last_error = "Invalid logger file parameters";
            return false;
        }
    }
    else if (strcmp(config_type, "monitor") == 0) {
        if (!config["monitors"].is<JsonArray>()) {
            m_last_error = "Invalid monitor configuration format";
            return false;
        }
        
        JsonArray monitors = config["monitors"].as<JsonArray>();
        for (JsonObject monitor_config : monitors) {
            if (!monitor_config["id"].is<const char*>() ||
                !monitor_config["label"].is<const char*>() ||
                !monitor_config["period_ms"].is<uint32_t>() ||
                !monitor_config["history_size"].is<uint8_t>()) {
                m_last_error = "Invalid monitor array entry format";
                return false;
            }
        }
    }
    else if (strcmp(config_type, "network") == 0) {
        if (!config["hostname"].is<const char*>() ||
            !config["http_port"].is<uint16_t>() ||
            !config["connection_timeout"].is<uint32_t>()) {
            m_last_error = "Invalid network configuration format";
            return false;
        }
        
        uint16_t port = config["http_port"].as<uint16_t>();
        if (port < 1 || port > 65535) {
            m_last_error = "Invalid HTTP port number";
            return false;
        }
    }
    else if (strcmp(config_type, "hardware") == 0) {
        if (!config["pins"].is<JsonObject>()) {
            m_last_error = "Invalid hardware configuration format";
            return false;
        }
        
        JsonObject pins = config["pins"].as<JsonObject>();
        if (!pins["display"].is<JsonObject>() ||
            !pins["sd"].is<JsonObject>() ||
            !pins["led"].is<JsonObject>()) {
            m_last_error = "Missing required pin configurations";
            return false;
        }
    }
    
    return true;
}

bool ConfigurationManager::reset_to_defaults() {
    if (!m_initialized) {
        m_last_error = "Configuration manager not initialized";
        return false;
    }

    // Remove all existing config files
    SD.remove(TIMING_CONFIG_FILE);
    SD.remove(SIGNAL_CONFIG_FILE);
    SD.remove(HARDWARE_CONFIG_FILE);
    SD.remove(DISPLAY_CONFIG_FILE);
    SD.remove(ALERT_CONFIG_FILE);
    SD.remove(NETWORK_CONFIG_FILE);
    SD.remove(MONITOR_CONFIG_FILE);
    SD.remove(SOUND_SENSOR_CONFIG_FILE);
    SD.remove(LED_CONFIG_FILE);
    SD.remove(LOGGER_CONFIG_FILE);

    // Clear cache and reload with defaults
    m_config_cache.clear();
    return load_all_configs();
}

void ConfigurationManager::set_defaults() {
    m_config_cache.clear();
    
    // Parse all default configurations
    deserializeJson(m_config_cache["timing"], config::defaults::TIMING_CONFIG);
    deserializeJson(m_config_cache["signal_processing"], config::defaults::SIGNAL_PROCESSING_CONFIG);
    deserializeJson(m_config_cache["display"], config::defaults::DISPLAY_CONFIG);
    deserializeJson(m_config_cache["alert"], config::defaults::ALERT_CONFIG);
    deserializeJson(m_config_cache["led"], config::defaults::LED_CONFIG);
    deserializeJson(m_config_cache["sound_sensor"], config::defaults::SOUND_SENSOR_CONFIG);
    deserializeJson(m_config_cache["logger"], config::defaults::LOGGER_CONFIG);
    deserializeJson(m_config_cache["monitor"], config::defaults::MONITOR_CONFIG);
}

bool ConfigurationManager::save_all_configs() {
    bool success = true;
    
    // Use as<JsonObject>() instead of to<JsonObject>()
    JsonObject timing = m_config_cache["timing"].as<JsonObject>();
    JsonObject signal = m_config_cache["signal_processing"].as<JsonObject>();
    JsonObject hardware = m_config_cache["hardware"].as<JsonObject>();
    JsonObject display = m_config_cache["display"].as<JsonObject>();
    JsonObject alert = m_config_cache["alert"].as<JsonObject>();
    JsonObject network = m_config_cache["network"].as<JsonObject>();
    JsonObject monitor = m_config_cache["monitor"].as<JsonObject>();
    JsonObject sound = m_config_cache["sound_sensor"].as<JsonObject>();
    JsonObject led = m_config_cache["led"].as<JsonObject>();
    JsonObject logger = m_config_cache["logger"].as<JsonObject>();
    
    success &= save_config_file(TIMING_CONFIG_FILE, timing);
    success &= save_config_file(SIGNAL_CONFIG_FILE, signal);
    success &= save_config_file(HARDWARE_CONFIG_FILE, hardware);
    success &= save_config_file(DISPLAY_CONFIG_FILE, display);
    success &= save_config_file(ALERT_CONFIG_FILE, alert);
    success &= save_config_file(NETWORK_CONFIG_FILE, network);
    success &= save_config_file(MONITOR_CONFIG_FILE, monitor);
    success &= save_config_file(SOUND_SENSOR_CONFIG_FILE, sound);
    success &= save_config_file(LED_CONFIG_FILE, led);
    success &= save_config_file(LOGGER_CONFIG_FILE, logger);
    
    return success;
}
 