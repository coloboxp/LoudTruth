#pragma once

#include <WebServer.h>
#include <ArduinoJson.h>
#include "signal_processor.hpp"
#include "display_manager.hpp"
#include "config/config.h"

class ApiServer
{
public:
    // Singleton pattern
    static ApiServer &instance()
    {
        static ApiServer instance;
        return instance;
    }

    void begin();
    void handle();
    bool is_available() const
    {
        return WiFi.status() == WL_CONNECTED;
    }

    // Resource setters
    void set_signal_processor(SignalProcessor *signal_processor)
    {
        m_signal_processor_ptr = signal_processor;
    }

    void set_display_manager(DisplayManager *display_manager)
    {
        m_display_manager_ptr = display_manager;
    }

private:
    ApiServer() = default;
    WebServer m_server{80};
    SignalProcessor *m_signal_processor_ptr{nullptr};
    DisplayManager *m_display_manager_ptr{nullptr};

    // Add declarations for the new methods
    void apply_default_config();
    void save_current_config();

    // REST API Endpoints
    void handle_root();
    void handle_get_status();
    void handle_get_monitors();
    void handle_get_monitor();
    void handle_post_monitor();
    void handle_put_monitor();
    void handle_delete_monitor();
    void handle_get_config();
    void handle_put_config();
    void handle_get_system_info();

    // Helper methods
    void send_json_response(JsonDocument &doc, int code = 200);
    void send_error(const char *message, int code = 400);
    bool validate_monitor_config(const JsonObject &config);
    void handle_not_found();
    void cors_headers();
    bool load_saved_config();

    void send_error_response(int code, const char* message);
    void send_success_response();
    bool parse_monitor_config(StatisticsMonitor::Config& cfg);
};