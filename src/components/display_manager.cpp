#include "display_manager.hpp"
#include <esp_log.h>

static const char* TAG = "DisplayManager";

/**
 * @brief Constructor for the DisplayManager class.
 */
DisplayManager::DisplayManager(const AlertManager& alert_manager)
    : m_display(U8G2_R0, U8X8_PIN_NONE, U8X8_PIN_NONE, U8X8_PIN_NONE)
    , m_alert_manager(alert_manager)
{
}

/**
 * @brief Initialize the display.
 */
void DisplayManager::begin() {
    m_display.begin();
    pinMode(config::hardware::pins::display::BACKLIGHT, OUTPUT);
    set_backlight(true);
    
    load_config();
    apply_display_settings();
    
    m_initialized = true;
}

void DisplayManager::begin(const JsonObject& config) {
    m_display.begin();
    pinMode(config::hardware::pins::display::BACKLIGHT, OUTPUT);
    set_backlight(true);
    
    if (!update_config(config)) {
        ESP_LOGW(TAG, "Invalid display configuration, using defaults");
        load_config();
    }
    
    apply_display_settings();
    m_initialized = true;
}

void DisplayManager::load_config() {
    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();
    
    if (ConfigurationManager::instance().get_display_config(config)) {
        update_config(config);
    } else {
        ESP_LOGW(TAG, "Failed to load display configuration, using defaults");
        // Default values are already set in the header
    }
}

bool DisplayManager::update_config(const JsonObject& config) {
    if (!validate_config(config)) {
        return false;
    }

    m_plot_points = config["plot_points"] | config::display::plot::PLOT_POINTS;
    m_plot_height = config["plot_height"] | config::display::plot::PLOT_HEIGHT;
    m_plot_baseline_y = config["plot_baseline_y"] | config::display::plot::PLOT_BASELINE_Y_POSITION;
    m_backlight_timeout_ms = config["backlight_timeout_ms"] | 5000;
    m_auto_backlight = config["auto_backlight"] | true;
    m_contrast = config["contrast"] | 128;
    m_flip_display = config["flip_display"] | false;

    if (m_initialized) {
        apply_display_settings();
    }

    return true;
}

void DisplayManager::apply_display_settings() {
    m_display.setContrast(m_contrast);
    if (m_flip_display) {
        m_display.setDisplayRotation(U8G2_R2);
    } else {
        m_display.setDisplayRotation(U8G2_R0);
    }
}

bool DisplayManager::validate_config(const JsonObject& config) {
    if (config["plot_points"].is<uint8_t>() && 
        (config["plot_points"].as<uint8_t>() < 10 || 
         config["plot_points"].as<uint8_t>() > 128)) {
        return false;
    }

    if (config["plot_height"].is<uint8_t>() && 
        (config["plot_height"].as<uint8_t>() < 10 || 
         config["plot_height"].as<uint8_t>() > 64)) {
        return false;
    }

    if (config["plot_baseline_y"].is<uint8_t>() && 
        config["plot_baseline_y"].as<uint8_t>() > 64) {
        return false;
    }

    if (config["contrast"].is<uint8_t>() && 
        config["contrast"].as<uint8_t>() > 255) {
        return false;
    }

    return true;
}

/**
 * @brief Update the display with the current signal processor values.
 * @param signal_processor The signal processor instance.
 */
void DisplayManager::update(const SignalProcessor& signal_processor) {
    if (!m_initialized) return;

    m_display.clearBuffer();
    
    draw_header(signal_processor);
    draw_plot();
    draw_stats(signal_processor);
    draw_alert_status();
    
    m_display.sendBuffer();

    // Handle auto backlight
    if (m_auto_backlight && m_backlight_active) {
        static uint32_t last_activity = millis();
        if (signal_processor.get_noise_category() > SignalProcessor::NoiseLevel::OK) {
            last_activity = millis();
        } else if (millis() - last_activity > m_backlight_timeout_ms) {
            set_backlight(false);
        }
    }
}

/**
 * @brief Add a point to the plot buffer.
 * @param value The value to add to the plot buffer.
 */
void DisplayManager::add_plot_point(float value) {
    if (!m_initialized) return;

    // Scale value to plot height
    int scaled_value = map(value * 100, 0, 100, 0, m_plot_height);
    m_plot_buffer.push(scaled_value);
}

/**
 * @brief Draw the statistics on the display.
 * @param signal_processor The signal processor instance.
 */
void DisplayManager::draw_stats(const SignalProcessor& signal_processor) {
    char buf[32];
    m_display.setFont(u8g2_font_5x7_tf);
    
    // Draw baseline value
    snprintf(buf, sizeof(buf), "Base: %.1f", signal_processor.get_baseline());
    m_display.drawStr(0, 62, buf);
    
    // Draw noise category
    const char* category;
    switch (signal_processor.get_noise_category()) {
        case SignalProcessor::NoiseLevel::OK:
            category = "OK";
            break;
        case SignalProcessor::NoiseLevel::REGULAR:
            category = "Regular";
            break;
        case SignalProcessor::NoiseLevel::ELEVATED:
            category = "Elevated";
            break;
        case SignalProcessor::NoiseLevel::CRITICAL:
            category = "Critical";
            break;
        default:
            category = "Unknown";
    }
    m_display.drawStr(64, 62, category);
}

void DisplayManager::draw_header(const SignalProcessor& signal_processor) {
    m_display.setFont(u8g2_font_6x10_tf);
    m_display.drawStr(0, 8, "Noise Monitor");
    
    // Draw current value
    char buf[10];
    snprintf(buf, sizeof(buf), "%.1f", signal_processor.get_current_value());
    m_display.drawStr(64, 8, buf);
}

/**
 * @brief Draw the plot on the display.
 */
void DisplayManager::draw_plot() {
    const uint8_t x_start = 0;
    const uint8_t y_start = m_plot_baseline_y;
    
    // Draw baseline
    m_display.drawHLine(x_start, y_start, m_plot_points);
    
    // Draw plot points
    for (uint8_t i = 0; i < m_plot_buffer.size(); i++) {
        uint8_t x = x_start + i;
        int value = m_plot_buffer[i];
        if (value > 0) {
            m_display.drawVLine(x, y_start - value, value);
        }
    }
}

void DisplayManager::draw_alert_status() {
    // Implementation depends on AlertManager public interface
    // Add alert status visualization as needed
}

/**
 * @brief Convert a noise level to a string.
 * @param level The noise level to convert.
 * @return The noise level as a string.
 */
const char *DisplayManager::noise_level_to_string(SignalProcessor::NoiseLevel level) const
{
    switch (level)
    {
    case SignalProcessor::NoiseLevel::OK:
        return "OK";
    case SignalProcessor::NoiseLevel::REGULAR:
        return "REG";
    case SignalProcessor::NoiseLevel::ELEVATED:
        return "HIGH";
    case SignalProcessor::NoiseLevel::CRITICAL:
        return "CRIT";
    default:
        return "???";
    }
}

void DisplayManager::control_backlight(SignalProcessor::NoiseLevel level) {
    uint32_t current_time = millis();
    
    if (level >= SignalProcessor::NoiseLevel::ELEVATED) {
        m_display.setPowerSave(0);  // Turn on display
        m_last_backlight_on = current_time;
    } else if (m_auto_backlight && 
              (current_time - m_last_backlight_on >= BACKLIGHT_TIMEOUT_MS)) {
        m_display.setPowerSave(1);  // Turn off display
    }
}

void DisplayManager::set_backlight(bool state)
{
    m_backlight_active = state;
    digitalWrite(config::hardware::pins::display::BACKLIGHT, state ? LOW : HIGH); // LOW = ON
}
