#include "display_manager.hpp"

/**
 * @brief Constructor for the DisplayManager class.
 */
DisplayManager::DisplayManager(const AlertManager &alert_manager)
    : m_alert_manager(alert_manager),
      m_u8g2(U8G2_R0, /* cs=*/5, /* dc=*/17, /* reset=*/16)
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
    m_u8g2.setFont(u8g2_font_4x6_tr); // Smallest font for maximum data

    draw_status_bar(signal_processor);
    draw_monitor_stats(signal_processor);
    draw_detailed_trends(signal_processor);
}

void DisplayManager::draw_status_bar(const SignalProcessor &signal_processor)
{
    char buffer[32];
    const char *level_str = noise_level_to_string(signal_processor.get_noise_category());

    float current = signal_processor.get_current_value();
    float baseline = signal_processor.get_baseline();
    char trend_arrow = (current > baseline * 1.1f) ? '^' : (current < baseline * 0.9f) ? 'v'
                                                                                       : '-';

    snprintf(buffer, sizeof(buffer), "%s %c %.0f/%.0f",
             level_str, trend_arrow, current, baseline);

    // Inverse video for status bar
    uint8_t width = m_u8g2.getStrWidth(buffer);
    m_u8g2.setDrawColor(1);
    m_u8g2.drawBox(0, 0, m_u8g2.getWidth(), 6);
    m_u8g2.setDrawColor(0);
    m_u8g2.drawStr(1, 5, buffer);
    m_u8g2.setDrawColor(1);
}

void DisplayManager::draw_monitor_stats(const SignalProcessor &signal_processor)
{
    std::vector<StatisticsMonitor *> priority_monitors = signal_processor.get_priority_monitors(2);
    char buffer[32];
    int y_pos = 8; // Start after status bar

    for (const auto *monitor : priority_monitors)
    {
        if (!monitor)
            continue;

        const auto &stats = monitor->get_stats();
        const auto &config = monitor->get_config();

        // Draw mini sparkline (24px wide, 8px high)
        draw_mini_sparkline(monitor, 0, y_pos, 24, 8);

        // Draw label and stats
        snprintf(buffer, sizeof(buffer), "%s L%.0f A%.0f H%.0f",
                 config.label.c_str(), stats.min, stats.avg, stats.max);
        m_u8g2.drawStr(26, y_pos + 6, buffer);

        // Draw trend indicator
        float trend = calculate_trend(monitor);
        const char *trend_char = trend > 0.05f ? "^" : trend < -0.05f ? "v"
                                                                      : "-";
        m_u8g2.drawStr(m_u8g2.getWidth() - 6, y_pos + 6, trend_char);

        y_pos += 12;
    }
}

void DisplayManager::draw_mini_sparkline(const StatisticsMonitor *monitor,
                                         int x, int y, int w, int h)
{
    if (!monitor)
        return;

    const auto &history = monitor->get_stats().history;
    if (history.empty())
        return;

    float min_val = history[0];
    float max_val = history[0];

    for (float val : history)
    {
        min_val = std::min(min_val, val);
        max_val = std::max(max_val, val);
    }

    float range = std::max(max_val - min_val, 1.0f);

    // Draw frame
    m_u8g2.drawFrame(x, y, w, h);

    // Draw sparkline
    int last_x = x;
    int last_y = y + h - 1;
    bool first = true;

    for (size_t i = 0; i < history.size(); ++i)
    {
        int plot_x = x + (i * (w - 2)) / history.size() + 1;
        int plot_y = y + h - 1 - ((history[i] - min_val) * (h - 2)) / range;

        if (!first)
        {
            m_u8g2.drawLine(last_x, last_y, plot_x, plot_y);
        }

        last_x = plot_x;
        last_y = plot_y;
        first = false;
    }
}

void DisplayManager::draw_detailed_trends(const SignalProcessor &signal_processor)
{
    std::vector<StatisticsMonitor *> priority_monitors = signal_processor.get_priority_monitors(2);
    if (priority_monitors.empty())
        return;

    // Draw trend area frame
    m_u8g2.drawFrame(0, 32, m_u8g2.getWidth(), 32);

    // Draw grid lines
    for (int i = 1; i < 3; i++)
    {
        m_u8g2.drawHLine(1, 32 + (i * 10), m_u8g2.getWidth() - 2);
        m_u8g2.drawVLine(i * (m_u8g2.getWidth() / 3), 33, 30);
    }

    // Draw trends
    if (priority_monitors.size() > 1)
    {
        draw_trend_line(priority_monitors[0], 33, 62, true);  // Primary trend
        draw_trend_line(priority_monitors[1], 33, 62, false); // Secondary trend
    }
    else
    {
        draw_trend_line(priority_monitors[0], 33, 62, true); // Single trend
    }
}

float DisplayManager::calculate_trend(const StatisticsMonitor *monitor)
{
    if (!monitor)
        return 0.0f;

    const auto &history = monitor->get_stats().history;
    if (history.size() < 2)
        return 0.0f;

    size_t count = std::min(size_t(5), history.size());
    float sum_start = 0, sum_end = 0;

    for (size_t i = 0; i < count; i++)
    {
        sum_start += history[i];
        sum_end += history[history.size() - 1 - i];
    }

    return (sum_end - sum_start) / (count * count);
}

void DisplayManager::draw_trend_line(const StatisticsMonitor *monitor,
                                     int y_start, int y_end, bool primary)
{
    if (!monitor)
        return;

    const auto &history = monitor->get_stats().history;
    if (history.empty())
        return;

    float min_val = history[0];
    float max_val = history[0];

    for (float val : history)
    {
        min_val = std::min(min_val, val);
        max_val = std::max(max_val, val);
    }

    float range = std::max(max_val - min_val, 1.0f);
    int height = y_end - y_start;

    // Draw points
    int last_x = 0;
    int last_y = 0;
    bool first = true;

    for (size_t i = 0; i < history.size(); ++i)
    {
        int x = (i * (m_u8g2.getWidth() - 4)) / history.size() + 2;
        int y = y_end - ((history[i] - min_val) * height) / range;

        if (!first)
        {
            if (primary)
            {
                m_u8g2.drawLine(last_x, last_y, x, y);
            }
            else
            {
                draw_dotted_line(last_x, last_y, x, y);
            }
        }

        last_x = x;
        last_y = y;
        first = false;
    }
}

void DisplayManager::draw_dotted_line(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1), sx = x1 < x2 ? 1 : -1;
    int dy = abs(y2 - y1), sy = y1 < y2 ? 1 : -1;
    int err = (dx > dy ? dx : -dy) / 2;
    int dot_counter = 0;

    while (true)
    {
        if (dot_counter++ % 2 == 0)
        {
            m_u8g2.drawPixel(x1, y1);
        }
        if (x1 == x2 && y1 == y2)
            break;
        int e2 = err;
        if (e2 > -dx)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 < dy)
        {
            err += dx;
            y1 += sy;
        }
    }
}

/**
 * @brief Draw the plot on the display.
 */
void DisplayManager::draw_plot()
{
    // This can remain as is or be modified to show longer-term trends
    // depending on your preference
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
