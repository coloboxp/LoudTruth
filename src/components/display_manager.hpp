#pragma once

#include <U8g2lib.h>
#include <CircularBuffer.hpp>
#include "alert_manager.hpp"
#include "signal_processor.hpp"
#include "config/configuration_manager.hpp"

class DisplayManager {
public:
    explicit DisplayManager(const AlertManager& alert_manager);
    
    void begin();
    void begin(const JsonObject& config);
    bool update_config(const JsonObject& config);
    void update(const SignalProcessor& signal_processor);
    void add_plot_point(float value);
    bool get_backlight_active() const { return m_backlight_active; }
    void set_backlight(bool state);
    const char* noise_level_to_string(SignalProcessor::NoiseLevel level) const;
    void control_backlight(SignalProcessor::NoiseLevel level);

private:
    static constexpr uint32_t BACKLIGHT_TIMEOUT_MS = 30000;  // 30 seconds timeout
    U8G2_SSD1306_128X64_NONAME_F_HW_I2C m_display;  // Add the display member
    const AlertManager& m_alert_manager;
    CircularBuffer<int, config::display::plot::PLOT_POINTS> m_plot_buffer;
    bool m_backlight_active{true};
    bool m_initialized{false};

    // Configurable display parameters
    uint8_t m_plot_points{config::display::plot::PLOT_POINTS};
    uint8_t m_plot_height{config::display::plot::PLOT_HEIGHT};
    uint8_t m_plot_baseline_y{config::display::plot::PLOT_BASELINE_Y_POSITION};
    uint16_t m_backlight_timeout_ms{5000};
    bool m_auto_backlight{true};
    uint8_t m_contrast{128};
    bool m_flip_display{false};

    // Add missing member variable
    uint32_t m_last_backlight_on{0};

    void draw_header(const SignalProcessor& signal_processor);
    void draw_plot();
    void draw_stats(const SignalProcessor& signal_processor);
    void draw_alert_status();
    bool validate_config(const JsonObject& config);
    void load_config();
    void apply_display_settings();
};