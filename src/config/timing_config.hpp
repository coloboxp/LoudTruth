#pragma once

#include <cstdint>

namespace config {
namespace timing {

class TimingConfig {
public:
    static const uint32_t DEFAULT_SAMPLE_INTERVAL;        // 20Hz sampling
    static const uint32_t DEFAULT_DISPLAY_INTERVAL;       // 10Hz display refresh
    static const uint32_t DEFAULT_LOGGING_INTERVAL;       // 1Hz logging
    static const uint32_t DEFAULT_LED_UPDATE_INTERVAL;    // 20Hz LED updates
};

} // namespace timing
} // namespace config 