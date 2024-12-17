#include "led_indicator.hpp"
#include <esp_log.h>

static const char* TAG = "LedIndicator";

LedIndicator::LedIndicator()
    : m_strip(config::led::NUM_PIXELS, 
              config::hardware::pins::led::DATA, 
              NEO_GRB + NEO_KHZ800)
{
    // Initialize default colors
    memcpy(m_colors, config::led::colors::INDICATOR_COLORS, 
           sizeof(uint32_t) * config::led::NUM_PIXELS);
}

void LedIndicator::begin() {
    m_strip.begin();
    load_config();
    m_strip.setBrightness(m_brightness);
    m_strip.show();
    m_initialized = true;
}

void LedIndicator::begin(const JsonObject& config) {
    m_strip.begin();
    if (!update_config(config)) {
        ESP_LOGW(TAG, "Invalid LED configuration, using defaults");
        load_config();
    }
    m_strip.setBrightness(m_brightness);
    m_strip.show();
    m_initialized = true;
}

void LedIndicator::load_config() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_led_config(config)) {
        update_config(config);
    } else {
        ESP_LOGW(TAG, "Failed to load LED configuration, using defaults");
        // Default values are already set in constructor and header
    }
}

bool LedIndicator::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    if (config["brightness"].is<uint8_t>()) {
        m_brightness = config["brightness"].as<uint8_t>();
        if (m_initialized) {
            m_strip.setBrightness(m_brightness);
        }
    }

    if (config["colors"].is<JsonArray>()) {
        JsonArray colors = config["colors"];
        size_t i = 0;
        for (JsonObject color : colors) {
            if (i >= m_num_pixels) break;
            
            if (color["r"].is<uint8_t>() && 
                color["g"].is<uint8_t>() && 
                color["b"].is<uint8_t>()) {
                m_colors[i] = make_color(
                    color["r"].as<uint8_t>(),
                    color["g"].as<uint8_t>(),
                    color["b"].as<uint8_t>()
                );
            }
            i++;
        }
    }

    m_reverse_direction = config["reverse_direction"] | false;
    m_animation_mode = config["animation_mode"] | 0;
    m_animation_speed = config["animation_speed"] | 1000;

    return true;
}

bool LedIndicator::validate_config(const JsonObject& config) {
    if (config["brightness"].is<uint8_t>() && 
        config["brightness"].as<uint8_t>() > 255) {
        return false;
    }

    if (config["animation_mode"].is<uint8_t>() && 
        config["animation_mode"].as<uint8_t>() > 2) {
        return false;
    }

    if (config["animation_speed"].is<uint16_t>() && 
        (config["animation_speed"].as<uint16_t>() < 100 || 
         config["animation_speed"].as<uint16_t>() > 5000)) {
        return false;
    }

    return true;
}

void LedIndicator::update(const SignalProcessor& signal_processor) {
    if (!m_initialized) return;

    float current_value = signal_processor.get_current_value();
    float baseline = signal_processor.get_baseline();
    float ratio = current_value / baseline;

    // Calculate number of active LEDs based on noise level
    uint8_t active_leds = 0;
    if (ratio > 1.0f) {
        active_leds = map(ratio * 100, 100, 400, 1, m_num_pixels);
        active_leds = constrain(active_leds, 0, m_num_pixels);
    }

    update_strip(active_leds);
}

void LedIndicator::update_strip(uint8_t active_leds) {
    m_strip.clear();

    if (active_leds == 0) {
        m_strip.show();
        return;
    }

    apply_animation_effect(active_leds);
    m_strip.show();
}

void LedIndicator::apply_animation_effect(uint8_t active_leds) {
    static unsigned long last_update = 0;
    static float animation_phase = 0.0f;
    
    unsigned long current_time = millis();
    float time_factor = (float)(current_time - last_update) / m_animation_speed;
    
    switch (m_animation_mode) {
        case 1: // Breathing effect
            {
                float brightness_factor = (sin(animation_phase) + 1.0f) / 2.0f;
                uint8_t temp_brightness = m_brightness * brightness_factor;
                m_strip.setBrightness(temp_brightness);
            }
            break;
            
        case 2: // Wave effect
            {
                for (uint8_t i = 0; i < active_leds; i++) {
                    float wave_factor = sin(animation_phase + (float)i / active_leds * 2 * PI);
                    wave_factor = (wave_factor + 1.0f) / 2.0f;
                    
                    uint32_t color = m_colors[m_reverse_direction ? m_num_pixels - 1 - i : i];
                    uint8_t r = ((color >> 16) & 0xFF) * wave_factor;
                    uint8_t g = ((color >> 8) & 0xFF) * wave_factor;
                    uint8_t b = (color & 0xFF) * wave_factor;
                    
                    m_strip.setPixelColor(i, r, g, b);
                }
                break;
            }
            
        default: // Static display
            for (uint8_t i = 0; i < active_leds; i++) {
                m_strip.setPixelColor(
                    i,
                    m_colors[m_reverse_direction ? m_num_pixels - 1 - i : i]
                );
            }
    }
    
    animation_phase += 2 * PI * time_factor;
    if (animation_phase >= 2 * PI) {
        animation_phase -= 2 * PI;
    }
    
    last_update = current_time;
}

void LedIndicator::set_brightness(uint8_t brightness) {
    m_brightness = brightness;
    m_strip.setBrightness(brightness);
    if (m_initialized) {
        m_strip.show();
    }
}

void LedIndicator::clear() {
    if (!m_initialized) return;
    m_strip.clear();
    m_strip.show();
}

uint32_t LedIndicator::make_color(uint8_t r, uint8_t g, uint8_t b) const {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}