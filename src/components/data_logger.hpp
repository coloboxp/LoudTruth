#pragma once

#include <SD.h>
#include <FS.h>
#include "signal_processor.hpp"
#include "config/configuration_manager.hpp"
#include <ArduinoJson.h>
#include <time.h>

class DataLogger {
public:
    DataLogger();
    
    bool begin();
    bool begin(const JsonObject& config);
    bool update_config(const JsonObject& config);
    void log_data(const SignalProcessor& signal_processor);
    bool is_logging_enabled() const { return m_logging_enabled && m_initialized; }
    const char* get_current_log_file() const { return m_current_log_file.c_str(); }

private:
    bool m_initialized{false};
    bool m_logging_enabled{true};
    std::string m_current_log_file;
    File m_log_file;
    time_t m_last_rotation_time{0};
    
    // Configurable parameters
    std::string m_log_directory{"/logs"};
    uint32_t m_max_file_size{1024 * 1024}; // 1MB default
    uint32_t m_rotation_interval{24 * 60 * 60}; // 24 hours in seconds
    uint8_t m_max_files{7}; // Keep a week's worth of logs
    bool m_include_timestamps{true};
    bool m_include_baseline{true};
    bool m_include_categories{true};
    uint8_t m_decimal_precision{2};
    
    void load_config();
    bool validate_config(const JsonObject& config);
    bool init_filesystem();
    bool create_log_directory();
    void rotate_logs();
    bool open_new_log_file();
    void cleanup_old_logs();
    std::string generate_filename() const;
    void write_log_header();
    std::string get_timestamp() const;
    std::string format_log_entry(const SignalProcessor& signal_processor) const;
};