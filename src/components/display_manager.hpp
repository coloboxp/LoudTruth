#pragma once

#include <U8g2lib.h>
#include "signal_processor.hpp"
#include "config/config.h"
#include "alert_manager.hpp"
#include "wifi_manager.hpp"
#include "api_handler.hpp"

/**
 * @brief Class representing the display manager.
 */
class DisplayManager
{
public:
    DisplayManager(const AlertManager &alert_manager);
    void begin();
    void update(const SignalProcessor &signal_processor);
    void add_plot_point(int value);

private:
    const AlertManager &m_alert_manager;
    U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI m_u8g2;
    int m_plot_buffer[config::display::plot::PLOT_POINTS];
    int m_plot_index{0};

    void draw_stats(const SignalProcessor &signal_processor);
    void draw_plot();
    const char *noise_level_to_string(SignalProcessor::NoiseLevel level) const;
    unsigned long m_last_backlight_on{0};
    bool m_backlight_active{false};

    void control_backlight(SignalProcessor::NoiseLevel level);
    void set_backlight(bool state);
};