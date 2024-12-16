#include "config.h"

namespace config
{
    namespace timing
    {
        uint32_t TimingConfig::sample_interval = TimingConfig::DEFAULT_SAMPLE_INTERVAL;
        uint32_t TimingConfig::display_interval = TimingConfig::DEFAULT_DISPLAY_INTERVAL;
        uint32_t TimingConfig::logging_interval = TimingConfig::DEFAULT_LOGGING_INTERVAL;
        uint32_t TimingConfig::led_update_interval = TimingConfig::DEFAULT_LED_UPDATE_INTERVAL;
    }

    namespace signal_processing
    {
        namespace ranges
        {
            int quiet = 100;    // Default value
            int moderate = 300; // Default value
            int loud = 600;     // Default value
            int max = 1000;     // Default value
        }
    }
}