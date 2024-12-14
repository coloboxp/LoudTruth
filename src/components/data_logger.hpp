#pragma once

#include <Arduino.h>
#include <SD.h>
#include "signal_processor.hpp"
#include "config/config.h"

/**
 * @brief Class representing the data logger.
 */
class DataLogger
{
public:
    DataLogger();
    bool begin();
    bool log_data(const SignalProcessor &signal_processor);

private:
    bool m_initialized{false};
    uint16_t m_file_counter{0};  // For fallback filename generation

    String get_current_filename();
    bool create_headers(const String& filename);
};