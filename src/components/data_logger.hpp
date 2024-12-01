#pragma once

#include <SD.h>
#include "config/config.h"
#include "signal_processor.hpp"

class DataLogger
{
public:
    DataLogger();
    bool begin();
    bool log_data(const SignalProcessor &signal_processor);

private:
    static constexpr const char *FILENAME = "noise_log.csv";
    bool create_headers();
    bool m_initialized{false};
};