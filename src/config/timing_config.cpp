#include "config/timing_config.hpp"

namespace config {
namespace timing {

const uint32_t TimingConfig::DEFAULT_SAMPLE_INTERVAL = 50;        // 20Hz sampling
const uint32_t TimingConfig::DEFAULT_DISPLAY_INTERVAL = 100;      // 10Hz display refresh
const uint32_t TimingConfig::DEFAULT_LOGGING_INTERVAL = 1000;     // 1Hz logging
const uint32_t TimingConfig::DEFAULT_LED_UPDATE_INTERVAL = 50;    // 20Hz LED updates

} // namespace timing
} // namespace config 