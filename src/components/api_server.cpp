#include "api_server.hpp"
#include <SD.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/adc.h>
#include <esp_chip_info.h>
#include "config/configuration_manager.hpp"
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifndef APP_VERSION
#define APP_VERSION "1.0.0"
#endif

#ifndef ARDUINO_VERSION
#define ARDUINO_VERSION ESP.getSdkVersion()
#endif

#ifndef configMAX_TASKS
#define configMAX_TASKS 16
#endif

void ApiServer::begin()
{
    // Load saved configuration
    load_saved_config();

    // Define API routes
    m_server.on("/", HTTP_GET, [this]()
                { handle_root(); });

    // Status endpoints
    m_server.on("/api/status", HTTP_GET, [this]()
                { handle_get_status(); });

    // Monitor endpoints
    m_server.on("/api/monitors", HTTP_GET, [this]()
                { handle_get_monitors(); });
    m_server.on("/api/monitors", HTTP_POST, [this]()
                { handle_post_monitor(); });
    m_server.on("/api/monitor", HTTP_GET, [this]()
                { handle_get_monitor(); });
    m_server.on("/api/monitor", HTTP_PUT, [this]()
                { handle_put_monitor(); });
    m_server.on("/api/monitor", HTTP_DELETE, [this]()
                { handle_delete_monitor(); });

    // Config endpoints
    m_server.on("/api/config", HTTP_GET, [this]()
                { handle_get_config(); });
    m_server.on("/api/config", HTTP_PUT, [this]()
                { handle_put_config(); });

    // System endpoints
    m_server.on("/api/system", HTTP_GET, [this]() {
        handle_get_system_info();
    });

    // Configuration management endpoints
    m_server.on("/api/config/default", HTTP_POST, [this]() {
        apply_default_config();
    });

    m_server.on("/api/config/save", HTTP_POST, [this]() {
        save_current_config();
    });

    m_server.onNotFound([this]()
                        { handle_not_found(); });
    m_server.begin();
}

void ApiServer::handle()
{
    m_server.handleClient();
}

void ApiServer::handle_root()
{
    JsonDocument doc;
    doc["status"] = "online";
    doc["version"] = "1.0.0";
    auto endpoints = doc["endpoints"].to<JsonArray>();
    endpoints.add("/api/status");
    endpoints.add("/api/monitors");
    endpoints.add("/api/monitor");
    endpoints.add("/api/config");
    endpoints.add("/api/system");

    send_json_response(doc);
}

void ApiServer::handle_get_status()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    JsonDocument doc;
    auto status = doc.to<JsonObject>();
    status["noise"]["current"] = m_signal_processor_ptr->get_current_value();
    status["noise"]["baseline"] = m_signal_processor_ptr->get_baseline();
    status["noise"]["category"] = static_cast<int>(m_signal_processor_ptr->get_noise_category());

    send_json_response(doc);
}

void ApiServer::handle_get_monitors()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    JsonDocument doc;
    auto monitors = doc["monitors"].to<JsonArray>();

    for (const auto *monitor : m_signal_processor_ptr->get_priority_monitors(10))
    {
        if (monitor)
        {
            auto mon = monitors.add<JsonObject>();
            mon["id"] = monitor->get_id();
            mon["label"] = monitor->get_config().label;
            mon["period_ms"] = monitor->get_config().period_ms;
            mon["priority"] = monitor->get_config().priority;

            const auto &stats = monitor->get_stats();
            auto stats_obj = mon["stats"].to<JsonObject>();
            stats_obj["current"] = stats.current;
            stats_obj["min"] = stats.min;
            stats_obj["max"] = stats.max;
            stats_obj["avg"] = stats.avg;

            // Add history data
            auto history = stats_obj["history"].to<JsonArray>();
            for (const auto &value : stats.history)
            {
                history.add(value);
            }
        }
    }

    send_json_response(doc);
}

void ApiServer::handle_get_monitor()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    if (!m_server.hasArg("id"))
    {
        return send_error("Missing monitor ID", 400);
    }

    const char* monitor_id = m_server.arg("id").c_str();
    const StatisticsMonitor* monitor = m_signal_processor_ptr->get_monitor(monitor_id);
    
    if (!monitor)
    {
        return send_error("Monitor not found", 404);
    }

    JsonDocument doc;
    auto mon = doc.to<JsonObject>();
    mon["id"] = monitor->get_id();
    mon["label"] = monitor->get_config().label;
    mon["period_ms"] = monitor->get_config().period_ms;
    mon["priority"] = monitor->get_config().priority;

    const auto& stats = monitor->get_stats();
    auto stats_obj = mon["stats"].to<JsonObject>();
    stats_obj["current"] = stats.current;
    stats_obj["min"] = stats.min;
    stats_obj["max"] = stats.max;
    stats_obj["avg"] = stats.avg;

    auto history = stats_obj["history"].to<JsonArray>();
    for (const auto& value : stats.history)
    {
        history.add(value);
    }

    send_json_response(doc);
}

void ApiServer::handle_post_monitor()
{
    StatisticsMonitor::Config cfg;
    if (!parse_monitor_config(cfg)) {
        send_error_response(400, "Invalid monitor configuration");
        return;
    }

    m_signal_processor_ptr->add_monitor(cfg);
    send_success_response();
}

void ApiServer::handle_put_monitor()
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));
    if (error) {
        send_error_response(400, "Invalid JSON");
        return;
    }

    JsonObject config = doc.as<JsonObject>();
    m_signal_processor_ptr->update_monitor_config(config);
    send_success_response();
}

void ApiServer::handle_delete_monitor()
{
    String monitor_id = m_server.pathArg(0);
    if (monitor_id.isEmpty()) {
        send_error_response(400, "Monitor ID required");
        return;
    }

    m_signal_processor_ptr->remove_monitor(std::string(monitor_id.c_str()));
    send_success_response();
}

void ApiServer::handle_get_config()
{
    JsonDocument doc;
    
    // Create JsonObjects using as<JsonObject>()
    JsonObject timing = doc["timing"].as<JsonObject>();
    JsonObject signal = doc["signal_processing"].as<JsonObject>();
    JsonObject display = doc["display"].as<JsonObject>();
    JsonObject alert = doc["alert"].as<JsonObject>();
    JsonObject network = doc["network"].as<JsonObject>();
    JsonObject monitor = doc["monitor"].as<JsonObject>();

    // Get configurations
    ConfigurationManager::instance().get_timing_config(timing);
    ConfigurationManager::instance().get_signal_processing_config(signal);
    ConfigurationManager::instance().get_display_config(display);
    ConfigurationManager::instance().get_alert_config(alert);
    ConfigurationManager::instance().get_network_config(network);
    ConfigurationManager::instance().get_monitor_config(monitor);

    send_json_response(doc);
}

void ApiServer::handle_put_config() {
    if (!m_server.hasArg("plain")) {
        return send_error("Missing request body", 400);
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));
    if (error) {
        return send_error("Invalid JSON format", 400);
    }

    JsonDocument response;
    JsonArray updates = response["updated"].to<JsonArray>();
    bool needs_restart = false;
    
    // Handle timing configuration
    if (doc["timing"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_timing_config(doc["timing"])) {
            updates.add("timing");
            needs_restart = true; // Timing changes require restart
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }
    
    // Handle signal processing configuration
    if (doc["signal_processing"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_signal_processing_config(doc["signal_processing"])) {
            updates.add("signal_processing");
            if (m_signal_processor_ptr) {
                JsonObject config;
                if (ConfigurationManager::instance().get_signal_processing_config(config)) {
                    m_signal_processor_ptr->update_config(config);
                }
            }
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }
    
    // Handle display configuration
    if (doc["display"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_display_config(doc["display"])) {
            updates.add("display");
            if (m_display_manager_ptr) {
                JsonObject config;
                if (ConfigurationManager::instance().get_display_config(config)) {
                    m_display_manager_ptr->update_config(config);
                }
            }
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }
    
    // Handle alert configuration
    if (doc["alert"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_alert_config(doc["alert"])) {
            updates.add("alert");
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }
    
    // Handle network configuration
    if (doc["network"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_network_config(doc["network"])) {
            updates.add("network");
            needs_restart = true; // Network changes require restart
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }
    
    // Handle monitor configuration
    if (doc["monitor"].is<JsonObject>()) {
        if (ConfigurationManager::instance().update_monitor_config(doc["monitor"])) {
            updates.add("monitor");
            if (m_signal_processor_ptr) {
                JsonObject config;
                if (ConfigurationManager::instance().get_monitor_config(config)) {
                    m_signal_processor_ptr->update_monitor_config(config);
                }
            }
        } else {
            response["errors"].add(ConfigurationManager::instance().get_last_error());
        }
    }

    response["needs_restart"] = needs_restart;
    
    if (updates.size() > 0) {
        response["status"] = "Configuration updated successfully";
        if (needs_restart) {
            response["message"] = "Some changes require a device restart to take effect";
        }
    } else if (!response["errors"].size()) {
        response["status"] = "No configurations were updated";
    } else {
        response["status"] = "Configuration update failed";
    }

    send_json_response(response);
}

void ApiServer::send_json_response(JsonDocument &doc, int code)
{
    String output;
    serializeJson(doc, output);
    cors_headers();
    m_server.send(code, "application/json", output);
}

void ApiServer::send_error(const char *message, int code)
{
    JsonDocument doc;
    doc["error"] = message;
    doc["code"] = code;

    String response;
    serializeJson(doc, response);
    cors_headers();
    m_server.send(code, "application/json", response);
}

void ApiServer::handle_not_found()
{
    cors_headers();
    send_error("Endpoint not found", 404);
}

void ApiServer::cors_headers()
{
    m_server.sendHeader("Access-Control-Allow-Origin", "*");
    m_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    m_server.sendHeader("Access-Control-Allow-Headers", "Content-Type");
}

bool ApiServer::validate_monitor_config(const JsonObject &config)
{
    if (!config["id"].is<const char *>())
        return false;
    if (!config["label"].is<const char *>())
        return false;
    if (config["period_ms"].as<uint32_t>() < 1000)
        return false; // Minimum 1 second
    if (config["priority"].as<uint8_t>() > 99)
        return false; // Max priority 99
    return true;
}

void ApiServer::handle_get_system_info() {
    JsonDocument doc;
    auto info = doc.to<JsonObject>();
    
    // System configuration
    info["version"] = APP_VERSION;
    info["device_name"] = config::device::NAME;
    
    // Get chip information
    esp_chip_info_t chip_info;
    esp_chip_info(&chip_info);
    
    // Chip information
    info["chip"]["model"] = "ESP32-S3";
    info["chip"]["revision"] = chip_info.revision;
    info["chip"]["cores"] = chip_info.cores;
    info["chip"]["features"] = chip_info.features;

    // Memory information
    info["memory"]["heap_size"] = ESP.getHeapSize();
    info["memory"]["free_heap"] = ESP.getFreeHeap();
    info["memory"]["min_free_heap"] = ESP.getMinFreeHeap();
    info["memory"]["max_alloc_heap"] = ESP.getMaxAllocHeap();
    info["memory"]["psram_size"] = ESP.getPsramSize();
    info["memory"]["free_psram"] = ESP.getFreePsram();

    // Version information
    info["version"]["app"] = APP_VERSION;
    info["version"]["arduino"] = ARDUINO_VERSION;
    info["version"]["sdk"] = ESP.getSdkVersion();
    
    // Hardware configuration
    JsonObject hw = info["hardware"].to<JsonObject>();
    hw["cpu_freq"] = ESP.getCpuFreqMHz();
    
    JsonObject pins = hw["pins"].to<JsonObject>();
    pins["spi"]["mosi"] = config::hardware::pins::MOSI;
    pins["spi"]["miso"] = config::hardware::pins::MISO;
    pins["spi"]["sck"] = config::hardware::pins::SCK;
    
    pins["display"]["cs"] = config::hardware::pins::display::CS;
    pins["display"]["dc"] = config::hardware::pins::display::DC;
    pins["display"]["reset"] = config::hardware::pins::display::RESET;
    pins["display"]["backlight"] = config::hardware::pins::display::BACKLIGHT;
    
    pins["led_strip"] = config::hardware::pins::led::STRIP;
    pins["led_indicator"] = config::hardware::pins::LED_INDICATOR;
    pins["sound_sensor"] = config::hardware::pins::analog::SOUND_SENSOR;

    // Task information
    #if CONFIG_FREERTOS_VTASKLIST_INCLUDE_COREID
    TaskStatus_t *task_array;
    uint32_t total_runtime;
    uint32_t task_count = uxTaskGetNumberOfTasks();
    
    task_array = (TaskStatus_t*)pvPortMalloc(task_count * sizeof(TaskStatus_t));
    if (task_array != NULL) {
        task_count = uxTaskGetSystemState(task_array, task_count, &total_runtime);
        
        JsonArray tasks = info["tasks"].to<JsonArray>();
        for (uint32_t i = 0; i < task_count; i++) {
            JsonObject task = tasks.createNestedObject();
            task["name"] = task_array[i].pcTaskName;
            task["priority"] = task_array[i].uxCurrentPriority;
            task["stack_hwm"] = task_array[i].usStackHighWaterMark;
            task["state"] = task_array[i].eCurrentState;
            task["core_id"] = task_array[i].xCoreID;
            task["runtime_percent"] = (task_array[i].ulRunTimeCounter * 100.0f) / total_runtime;
        }
        vPortFree(task_array);
    }
    #endif

    send_json_response(doc);
}

void ApiServer::apply_default_config() {
    if (ConfigurationManager::instance().reset_to_defaults()) {
        JsonDocument doc;
        doc["status"] = "Default configuration applied";
        send_json_response(doc);
    } else {
        send_error("Failed to apply default configuration", 500);
    }
}

void ApiServer::save_current_config() {
    if (ConfigurationManager::instance().save_all_configs()) {
        send_success_response();
    } else {
        send_error_response(500, "Failed to save configuration");
    }
}

bool ApiServer::load_saved_config() {
    if (!ConfigurationManager::instance().is_initialized()) {
        ESP_LOGE("ApiServer", "Configuration manager not initialized");
        return false;
    }

    JsonDocument doc;
    JsonObject config = doc.to<JsonObject>();

    // Load timing configuration
    if (!ConfigurationManager::instance().get_timing_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load timing configuration");
        return false;
    }

    // Load signal processing configuration
    if (!ConfigurationManager::instance().get_signal_processing_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load signal processing configuration");
        return false;
    }

    // Load display configuration
    if (m_display_manager_ptr && !ConfigurationManager::instance().get_display_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load display configuration");
        return false;
    }

    // Load alert configuration
    if (!ConfigurationManager::instance().get_alert_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load alert configuration");
        return false;
    }

    // Load network configuration
    if (!ConfigurationManager::instance().get_network_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load network configuration");
        return false;
    }

    // Load monitor configuration
    if (m_signal_processor_ptr && !ConfigurationManager::instance().get_monitor_config(config)) {
        ESP_LOGW("ApiServer", "Failed to load monitor configuration");
        return false;
    }

    return true;
}

void ApiServer::send_error_response(int code, const char* message) {
    JsonDocument doc;
    doc["error"] = message;
    send_json_response(doc, code);
}

void ApiServer::send_success_response() {
    JsonDocument doc;
    doc["status"] = "success";
    send_json_response(doc);
}

bool ApiServer::parse_monitor_config(StatisticsMonitor::Config& cfg) {
    if (!m_server.hasArg("plain")) {
        return false;
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));
    if (error) {
        return false;
    }

    JsonObject config = doc.as<JsonObject>();
    cfg.id = config["id"] | "";
    cfg.label = config["label"] | "";
    cfg.period_ms = config["period_ms"] | 60000;
    cfg.priority = config["priority"] | 99;
    cfg.history_size = config["history_size"] | 60;

    return true;
}