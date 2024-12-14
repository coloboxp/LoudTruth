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
    if (!m_initialized)
    {
        return false;
    }

    String current_filename = get_current_filename();

    // Create headers if this is a new file
    if (!create_headers(current_filename))
    {
        return false;
    }

    File dataFile = SD.open(current_filename, FILE_WRITE);
    if (!dataFile)
    {
        return false;
    }

    // Get current timestamp
    time_t now;
    time(&now); // Get current time as Unix timestamp

    // Format: unix_timestamp,current_noise,baseline,category,1min_avg,15min_avg
    dataFile.print(now);
    dataFile.print(",");
    dataFile.print(signal_processor.get_current_value());
    dataFile.print(",");
    dataFile.print(signal_processor.get_baseline());
    dataFile.print(",");
    dataFile.print(static_cast<int>(signal_processor.get_noise_category()));
    dataFile.print(",");
    dataFile.print(signal_processor.get_one_min_stats().avg);
    dataFile.print(",");
    dataFile.println(signal_processor.get_fifteen_min_stats().avg);

    dataFile.close();
    return true;
}