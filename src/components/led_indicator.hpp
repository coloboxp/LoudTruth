#pragma once

#include <Adafruit_NeoPixel.h>
#include "signal_processor.hpp"
#include "config/configuration_manager.hpp"

class LedIndicator
{
public:
    LedIndicator();
    
    void begin();
    void begin(const JsonObject& config);
    bool update_config(const JsonObject& config);
    void update(const SignalProcessor& signal_processor);
    void set_brightness(uint8_t brightness);
    void clear();

private:
    Adafruit_NeoPixel m_strip;
    bool m_initialized{false};
    
    // Configurable parameters
    uint8_t m_num_pixels{config::led::NUM_PIXELS};
    uint8_t m_brightness{50};
    uint32_t m_colors[config::led::NUM_PIXELS];
    bool m_reverse_direction{false};
    uint8_t m_animation_mode{0}; // 0=static, 1=breathing, 2=wave
    uint16_t m_animation_speed{1000};
    
    void load_config();
    bool validate_config(const JsonObject& config);
    void update_strip(uint8_t active_leds);
    uint32_t get_interpolated_color(float position) const;
    void apply_animation_effect(uint8_t active_leds);
    uint32_t make_color(uint8_t r, uint8_t g, uint8_t b) const;
};