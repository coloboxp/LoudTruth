#pragma once

#include <U8g2lib.h>
#include "config/config.h"
#include "signal_processor.hpp"

/**
 * @brief Class representing the display manager.
 */
class DisplayManager
{
public:
    DisplayManager();
    void begin();
    void update(const SignalProcessor &signal_processor);
    void add_plot_point(int value);

private:
    U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI m_u8g2;
    int m_plot_buffer[display_config::plot::PLOT_POINTS];
    int m_plot_index{0};

    void draw_stats(const SignalProcessor &signal_processor);
    void draw_plot();
    const char *noise_level_to_string(SignalProcessor::NoiseLevel level);
};