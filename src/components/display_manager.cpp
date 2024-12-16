#include "display_manager.hpp"

/**
 * @brief Constructor for the DisplayManager class.
 */
DisplayManager::DisplayManager(const AlertManager &alert_manager)
    : m_u8g2(U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI(
          U8G2_R0,
          config::hardware::pins::display::CS,
          config::hardware::pins::display::DC,
          config::hardware::pins::display::RESET)),
      m_alert_manager(alert_manager)
{
    memset(m_plot_buffer, 0, sizeof(m_plot_buffer));
}

/**
 * @brief Initialize the display.
 */
void DisplayManager::begin()
{
    // Reset pin setup
    pinMode(config::hardware::pins::display::RESET, OUTPUT);
    digitalWrite(config::hardware::pins::display::RESET, HIGH);
    delay(1);
    digitalWrite(config::hardware::pins::display::RESET, LOW); // Reset display
    delay(10);
    digitalWrite(config::hardware::pins::display::RESET, HIGH);
    delay(100); // Wait for reset to complete

    // Initialize display
    if (!m_u8g2.begin())
    {
        Serial.println("Display init failed!");
        return;
    }
    Serial.println("Display initialized");

    // Display setup
    m_u8g2.setContrast(0x3A); // Using contrast from the GitHub issue

    // Initialize backlight pin and ensure it starts OFF
    pinMode(config::hardware::pins::display::BACKLIGHT, OUTPUT);
    set_backlight(false);

    // Test pattern
    m_u8g2.clearBuffer();
    m_u8g2.drawFrame(0, 0, m_u8g2.getWidth(), m_u8g2.getHeight());
    m_u8g2.setFont(u8g2_font_ncenB14_tr);
    m_u8g2.drawStr(0, 20, "Zoo Decibel");
    m_u8g2.sendBuffer();

    Serial.printf("Display dimensions: %dx%d\n", m_u8g2.getWidth(), m_u8g2.getHeight());
}

/**
 * @brief Update the display with the current signal processor values.
 * @param signal_processor The signal processor instance.
 */
void DisplayManager::update(const SignalProcessor &signal_processor)
{
    SignalProcessor::NoiseLevel current_level = signal_processor.get_noise_category();

    // Control backlight based on noise level
    control_backlight(current_level);

    // Always update display regardless of backlight state
    m_u8g2.clearBuffer();
    draw_stats(signal_processor);
    draw_plot();
    m_u8g2.sendBuffer();
}

/**
 * @brief Add a point to the plot buffer.
 * @param value The value to add to the plot buffer.
 */
void DisplayManager::add_plot_point(int value)
{
    m_plot_buffer[m_plot_index] = value;
    m_plot_index = (m_plot_index + 1) % config::display::plot::PLOT_POINTS;
}

/**
 * @brief Draw the statistics on the display.
 * @param signal_processor The signal processor instance.
 */
void DisplayManager::draw_stats(const SignalProcessor &signal_processor)
{
    m_u8g2.setFont(u8g2_font_6x10_tf);

    // Draw current noise level and category
    char noise_str[32];
    snprintf(noise_str, sizeof(noise_str), "ADC: %d",
             static_cast<int>(signal_processor.get_current_value()));
    m_u8g2.drawStr(0, 10, noise_str);

    // Draw WiFi and ThingSpeak status in top right
    char status[5] = "    "; // 4 spaces + null terminator
    if (wifi::WiFiManager::instance().is_connected())
    {
        status[0] = 'W';
    }
    if (ApiHandler::instance().is_available())
    {
        status[2] = 'T';
    }
    m_u8g2.drawStr(110, 10, status);

    // Draw noise category
    const char *category = noise_level_to_string(signal_processor.get_noise_category());
    if (signal_processor.get_noise_category() >= SignalProcessor::NoiseLevel::ELEVATED)
    {
        uint8_t category_width = m_u8g2.getStrWidth(category);
        m_u8g2.setDrawColor(1);
        m_u8g2.drawBox(64, 2, category_width, 10);
        m_u8g2.setDrawColor(0);
        m_u8g2.drawStr(64, 10, category);
        m_u8g2.setDrawColor(1);
    }
    else
    {
        m_u8g2.drawStr(64, 10, category);
    }

    // Draw statistics with proper alignment
    const auto &one_min = signal_processor.get_one_min_stats();
    const auto &fifteen_min = signal_processor.get_fifteen_min_stats();

    char stats_str[32];
    snprintf(stats_str, sizeof(stats_str), "1m:  %4d [%4d-%4d]",
             static_cast<int>(one_min.avg),
             one_min.min,
             one_min.max);
    m_u8g2.drawStr(0, 20, stats_str);

    snprintf(stats_str, sizeof(stats_str), "15m: %4d [%4d-%4d]",
             static_cast<int>(fifteen_min.avg),
             fifteen_min.min,
             fifteen_min.max);
    m_u8g2.drawStr(0, 30, stats_str);

    // // Draw chart grid and labels
    // m_u8g2.drawHLine(0, 40, 128); // Baseline
    // m_u8g2.drawVLine(0, 40, -30); // Y-axis
}

/**
 * @brief Draw the plot on the display.
 */
void DisplayManager::draw_plot()
{
    // Draw the actual plot points with increased sensitivity
    for (int i = 0; i < config::display::plot::PLOT_POINTS - 1; i++)
    {
        int x1 = i * 2;
        int x2 = (i + 1) * 2;

        // Shows the value of the plot buffer as a line on the display
        int y1 = map(m_plot_buffer[i], 0, config::signal_processing::ranges::MAX,
                     config::display::plot::PLOT_BASELINE_Y_POSITION,
                     config::display::plot::PLOT_BASELINE_Y_POSITION - config::display::plot::PLOT_HEIGHT);

        // Draws a line between the current and next point
        int y2 = map(m_plot_buffer[(i + 1)], 0, config::signal_processing::ranges::MAX,
                     config::display::plot::PLOT_BASELINE_Y_POSITION,
                     config::display::plot::PLOT_BASELINE_Y_POSITION - config::display::plot::PLOT_HEIGHT);

        // Draws the line between the current and next point
        m_u8g2.drawLine(x1, y1, x2, y2);
    }
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

void DisplayManager::control_backlight(SignalProcessor::NoiseLevel level)
{
    const unsigned long BACKLIGHT_TIMEOUT_MS = 30000; // 30 seconds timeout
    unsigned long current_time = millis();

    // Turn on backlight if noise level is above OK
    if (level > SignalProcessor::NoiseLevel::OK)
    {
        set_backlight(true);
        m_last_backlight_on = current_time;
    }
    // Turn off backlight if timeout has elapsed
    else if (m_backlight_active &&
             (current_time - m_last_backlight_on >= BACKLIGHT_TIMEOUT_MS))
    {
        set_backlight(false);
    }
}

void DisplayManager::set_backlight(bool state)
{
    m_backlight_active = state;
    digitalWrite(config::hardware::pins::display::BACKLIGHT, state ? LOW : HIGH); // LOW = ON
}
