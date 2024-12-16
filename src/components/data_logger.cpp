#include "data_logger.hpp"
#include <time.h>

DataLogger::DataLogger() = default;

/**
 * @brief Initialize the data logger.
 * @return True if initialization is successful, false otherwise.
 */
bool DataLogger::begin()
{
    if (!SD.begin(config::hardware::pins::sd::CS))
    {
        Serial.println("SD card initialization failed!");
        return false;
    }

    Serial.println("SD card initialized.");
    m_initialized = true;
    return true;
}

String DataLogger::get_current_filename()
{
    char filename[32];
    time_t now;
    time(&now);
    struct tm timeinfo;

    if (getLocalTime(&timeinfo))
    {
        // Format: YYMMDD.csv (e.g., 240315.csv for March 15, 2024)
        strftime(filename, sizeof(filename), "%y%m%d.csv", &timeinfo);
    }
    else
    {
        // Fallback if time is not set: use counter
        sprintf(filename, "LOG%03d.csv", m_file_counter++);
        if (m_file_counter > 999)
            m_file_counter = 0;
    }

    return String(filename);
}

/**
 * @brief Create the headers for the data file.
 * @return True if the headers are created successfully, false otherwise.
 */
bool DataLogger::create_headers(const String &filename)
{
    if (!m_initialized || SD.exists(filename))
    {
        return true;
    }

    File dataFile = SD.open(filename, FILE_WRITE);
    if (!dataFile)
    {
        return false;
    }

    // Use Unix timestamp in headers
    dataFile.println("timestamp,noise,baseline,category,1min_avg,15min_avg");
    dataFile.close();
    return true;
}

/**
 * @brief Log the data to the file.
 * @param signal_processor The signal processor instance.
 * @return True if the data is logged successfully, false otherwise.
 */
bool DataLogger::log_data(const SignalProcessor &signal_processor)
{
    File dataFile = SD.open("/data.csv", FILE_APPEND);
    if (!dataFile)
    {
        return false;
    }

    // Get monitors by ID
    auto *one_min = signal_processor.get_monitor("1min");
    auto *fifteen_min = signal_processor.get_monitor("15min");

    // Write current values and averages
    dataFile.print(signal_processor.get_current_value());
    dataFile.print(",");
    dataFile.print(one_min ? one_min->get_stats().avg : 0.0f);
    dataFile.print(",");
    dataFile.println(fifteen_min ? fifteen_min->get_stats().avg : 0.0f);

    dataFile.close();
    return true;
}