#include "data_logger.hpp"
#include <esp_log.h>

static const char* TAG = "DataLogger";

DataLogger::DataLogger() = default;

bool DataLogger::begin() {
    if (!init_filesystem()) {
        ESP_LOGE(TAG, "Failed to initialize filesystem");
        return false;
    }

    load_config();
    
    if (!create_log_directory()) {
        ESP_LOGE(TAG, "Failed to create log directory");
        return false;
    }

    cleanup_old_logs();
    if (!open_new_log_file()) {
        ESP_LOGE(TAG, "Failed to open new log file");
        return false;
    }

    m_initialized = true;
    return true;
}

bool DataLogger::begin(const JsonObject& config) {
    if (!init_filesystem()) {
        ESP_LOGE(TAG, "Failed to initialize filesystem");
        return false;
    }

    if (!update_config(config)) {
        ESP_LOGW(TAG, "Invalid logger configuration, using defaults");
        load_config();
    }

    if (!create_log_directory()) {
        ESP_LOGE(TAG, "Failed to create log directory");
        return false;
    }

    cleanup_old_logs();
    if (!open_new_log_file()) {
        ESP_LOGE(TAG, "Failed to open new log file");
        return false;
    }

    m_initialized = true;
    return true;
}

void DataLogger::load_config() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_logger_config(config)) {
        update_config(config);
    } else {
        ESP_LOGW(TAG, "Failed to load logger configuration, using defaults");
        // Default values are already set in the header
    }
}

bool DataLogger::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    m_logging_enabled = config["enabled"] | true;
    m_log_directory = config["directory"] | "/logs";
    m_max_file_size = config["max_file_size"] | (1024 * 1024);
    m_rotation_interval = config["rotation_interval"] | (24 * 60 * 60);
    m_max_files = config["max_files"] | 7;
    m_include_timestamps = config["include_timestamps"] | true;
    m_include_baseline = config["include_baseline"] | true;
    m_include_categories = config["include_categories"] | true;
    m_decimal_precision = config["decimal_precision"] | 2;

    return true;
}

bool DataLogger::validate_config(const JsonObject& config) {
    if (config["max_file_size"].is<uint32_t>()) {
        uint32_t size = config["max_file_size"].as<uint32_t>();
        if (size < 1024 || size > (10 * 1024 * 1024)) { // Between 1KB and 10MB
            return false;
        }
    }

    if (config["rotation_interval"].is<uint32_t>()) {
        uint32_t interval = config["rotation_interval"].as<uint32_t>();
        if (interval < 300 || interval > (7 * 24 * 60 * 60)) { // Between 5 minutes and 1 week
            return false;
        }
    }

    if (config["max_files"].is<uint8_t>()) {
        uint8_t files = config["max_files"].as<uint8_t>();
        if (files < 1 || files > 31) { // Maximum one month of logs
            return false;
        }
    }

    if (config["decimal_precision"].is<uint8_t>()) {
        uint8_t precision = config["decimal_precision"].as<uint8_t>();
        if (precision > 6) { // Reasonable maximum precision
            return false;
        }
    }

    return true;
}

bool DataLogger::init_filesystem() {
    if (!SD.begin(config::hardware::pins::sd::CS)) {
        return false;
    }
    return true;
}

bool DataLogger::create_log_directory() {
    if (!SD.exists(m_log_directory.c_str())) {
        return SD.mkdir(m_log_directory.c_str());
    }
    return true;
}

void DataLogger::rotate_logs() {
    time_t now = time(nullptr);
    if ((now - m_last_rotation_time) >= m_rotation_interval || 
        m_log_file.size() >= m_max_file_size) {
        
        m_log_file.close();
        cleanup_old_logs();
        open_new_log_file();
        m_last_rotation_time = now;
    }
}

bool DataLogger::open_new_log_file() {
    m_current_log_file = generate_filename();
    m_log_file = SD.open(m_current_log_file.c_str(), FILE_WRITE);
    
    if (!m_log_file) {
        return false;
    }

    write_log_header();
    return true;
}

void DataLogger::cleanup_old_logs() {
    File root = SD.open(m_log_directory.c_str());
    if (!root || !root.isDirectory()) {
        return;
    }

    std::vector<std::string> log_files;
    
    File file = root.openNextFile();
    while (file) {
        if (!file.isDirectory()) {
            log_files.push_back(file.name());
        }
        file = root.openNextFile();
    }

    // Sort files by name (which includes timestamp)
    std::sort(log_files.begin(), log_files.end());

    // Remove oldest files if we exceed max_files
    while (log_files.size() > m_max_files) {
        std::string to_remove = m_log_directory + "/" + log_files.front();
        SD.remove(to_remove.c_str());
        log_files.erase(log_files.begin());
    }
}

std::string DataLogger::generate_filename() const {
    char filename[64];
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    snprintf(filename, sizeof(filename), "%s/noise_%04d%02d%02d_%02d%02d%02d.csv",
             m_log_directory.c_str(),
             timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
             timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
             
    return std::string(filename);
}

void DataLogger::write_log_header() {
    if (!m_log_file) return;

    std::string header = "value";
    if (m_include_timestamps) header += ",timestamp";
    if (m_include_baseline) header += ",baseline";
    if (m_include_categories) header += ",category";
    header += "\n";

    m_log_file.print(header.c_str());
    m_log_file.flush();
}

std::string DataLogger::get_timestamp() const {
    char timestamp[32];
    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(timestamp);
}

void DataLogger::log_data(const SignalProcessor& signal_processor) {
    if (!m_initialized || !m_logging_enabled || !m_log_file) {
        return;
    }

    rotate_logs();

    std::string log_entry = format_log_entry(signal_processor);
    m_log_file.print(log_entry.c_str());
    m_log_file.flush();
}

std::string DataLogger::format_log_entry(const SignalProcessor& signal_processor) const {
    char value_str[16];
    snprintf(value_str, sizeof(value_str), "%.*f", m_decimal_precision, signal_processor.get_current_value());
    
    std::string entry = value_str;
    
    if (m_include_timestamps) {
        entry += "," + get_timestamp();
    }
    
    if (m_include_baseline) {
        snprintf(value_str, sizeof(value_str), "%.*f", m_decimal_precision, signal_processor.get_baseline());
        entry += "," + std::string(value_str);
    }
    
    if (m_include_categories) {
        const char* category;
        switch (signal_processor.get_noise_category()) {
            case SignalProcessor::NoiseLevel::OK: category = "OK"; break;
            case SignalProcessor::NoiseLevel::REGULAR: category = "REGULAR"; break;
            case SignalProcessor::NoiseLevel::ELEVATED: category = "ELEVATED"; break;
            case SignalProcessor::NoiseLevel::CRITICAL: category = "CRITICAL"; break;
            default: category = "UNKNOWN";
        }
        entry += "," + std::string(category);
    }
    
    entry += "\n";
    return entry;
}