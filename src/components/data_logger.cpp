#include "data_logger.hpp"

DataLogger::DataLogger() = default;

bool DataLogger::begin()
{
    if (!SD.begin(pins::sd::CS))
    {
        Serial.println("SD card initialization failed!");
        return false;
    }

    Serial.println("SD card initialized.");
    m_initialized = true;

    return create_headers();
}

bool DataLogger::create_headers()
{
    if (!m_initialized || SD.exists(FILENAME))
    {
        return true;
    }

    File dataFile = SD.open(FILENAME, FILE_WRITE);
    if (!dataFile)
    {
        return false;
    }

    dataFile.println("timestamp,noise,baseline,category,1min_avg,15min_avg");
    dataFile.close();
    return true;
}

bool DataLogger::log_data(const SignalProcessor &signal_processor)
{
    if (!m_initialized)
    {
        return false;
    }

    File dataFile = SD.open(FILENAME, FILE_WRITE);
    if (!dataFile)
    {
        return false;
    }

    // Format: timestamp,current_noise,baseline,category,1min_avg,15min_avg
    dataFile.print(millis());
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