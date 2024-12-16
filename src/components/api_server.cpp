#include "api_server.hpp"
#include <SD.h>
#include <esp_ota_ops.h>
#include <esp_system.h>
#include <esp_timer.h>
#include <driver/adc.h>

#ifndef APP_VERSION
#define APP_VERSION "1.0.0"
#endif

#ifndef ARDUINO_VERSION
#define ARDUINO_VERSION ARDUINO_ESP32_VERSION
#endif

#ifndef configMAX_TASKS
#define configMAX_TASKS 16
#endif

void ApiServer::begin()
{
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

    String id = m_server.arg("id");
    if (id.isEmpty())
    {
        return send_error("Monitor ID required");
    }

    auto *monitor = m_signal_processor_ptr->get_monitor(id.c_str());
    if (!monitor)
    {
        return send_error("Monitor not found", 404);
    }

    JsonDocument doc;
    doc["id"] = monitor->get_id();
    doc["label"] = monitor->get_config().label;
    doc["period_ms"] = monitor->get_config().period_ms;
    doc["priority"] = monitor->get_config().priority;

    const auto &stats = monitor->get_stats();
    auto stats_obj = doc["stats"].to<JsonObject>();
    stats_obj["current"] = stats.current;
    stats_obj["min"] = stats.min;
    stats_obj["max"] = stats.max;
    stats_obj["avg"] = stats.avg;

    auto history = stats_obj["history"].to<JsonArray>();
    for (const auto &value : stats.history)
    {
        history.add(value);
    }

    send_json_response(doc);
}

void ApiServer::handle_post_monitor()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));

    if (error)
    {
        return send_error("Invalid JSON");
    }

    if (!validate_monitor_config(doc.as<JsonObject>()))
    {
        return send_error("Invalid monitor configuration");
    }

    StatisticsMonitor::Config config;
    config.id = doc["id"] | "monitor";
    config.label = doc["label"] | "Monitor";
    config.period_ms = doc["period_ms"] | 60000;
    config.priority = doc["priority"] | 99;
    config.history_size = doc["history_size"] | 60;

    m_signal_processor_ptr->add_monitor(config);

    JsonDocument response;
    response["status"] = "Monitor created";
    response["id"] = config.id;
    send_json_response(response, 201);
}

void ApiServer::handle_put_monitor()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));

    if (error)
    {
        return send_error("Invalid JSON");
    }

    const char *id = doc["id"];
    if (!id)
    {
        return send_error("Monitor ID required");
    }

    auto *monitor = m_signal_processor_ptr->get_monitor(id);
    if (!monitor)
    {
        return send_error("Monitor not found", 404);
    }

    if (!validate_monitor_config(doc.as<JsonObject>()))
    {
        return send_error("Invalid monitor configuration");
    }

    StatisticsMonitor::Config config = monitor->get_config();
    config.label = doc["label"] | config.label;
    config.period_ms = doc["period_ms"] | config.period_ms;
    config.priority = doc["priority"] | config.priority;
    config.history_size = doc["history_size"] | config.history_size;

    m_signal_processor_ptr->remove_monitor(id);
    m_signal_processor_ptr->add_monitor(config);

    JsonDocument response;
    response["status"] = "Monitor updated";
    response["id"] = id;
    send_json_response(response);
}

void ApiServer::handle_delete_monitor()
{
    if (!m_signal_processor_ptr)
    {
        return send_error("Signal processor not initialized", 500);
    }

    String id = m_server.arg("id");
    if (id.isEmpty())
    {
        return send_error("Monitor ID required");
    }

    m_signal_processor_ptr->remove_monitor(id.c_str());

    JsonDocument response;
    response["status"] = "Monitor deleted";
    response["id"] = id;
    send_json_response(response);
}

void ApiServer::handle_get_config()
{
    JsonDocument doc;
    // Add configuration parameters here
    doc["version"] = "1.0.0";
    // Add more config parameters as needed

    send_json_response(doc);
}

void ApiServer::handle_put_config()
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, m_server.arg("plain"));

    if (error)
    {
        return send_error("Invalid JSON");
    }

    // Update configuration parameters here
    // Add validation and update logic as needed

    JsonDocument response;
    response["status"] = "Configuration updated";
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
    m_server.send(code, "application/json", response);
}

void ApiServer::handle_not_found()
{
    send_error("Not found", 404);
}

void ApiServer::cors_headers()
{
    m_server.sendHeader("Access-Control-Allow-Origin", "*");
    m_server.sendHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    m_server.sendHeader("Access-Control-Allow-Headers", "Origin, X-Requested-With, Content-Type, Accept");
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

void ApiServer::handle_get_system_info()
{
    JsonDocument doc;

    // System Info
    doc["system"]["chip_model"] = ESP.getChipModel();
    doc["system"]["chip_cores"] = ESP.getChipCores();
    doc["system"]["chip_revision"] = ESP.getChipRevision();
    doc["system"]["sdk_version"] = ESP.getSdkVersion();

    // Memory Info
    doc["memory"]["heap_size"] = ESP.getHeapSize();
    doc["memory"]["free_heap"] = ESP.getFreeHeap();
    doc["memory"]["min_free_heap"] = ESP.getMinFreeHeap();
    doc["memory"]["max_alloc_heap"] = ESP.getMaxAllocHeap();
    doc["memory"]["free_dma"] = heap_caps_get_free_size(MALLOC_CAP_DMA);
    doc["memory"]["free_internal"] = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    doc["memory"]["heap_fragmentation"] = static_cast<int>(heap_caps_get_largest_free_block(MALLOC_CAP_INTERNAL) * 100.0 / ESP.getFreeHeap());

// PSRAM if available
#ifdef BOARD_HAS_PSRAM
    doc["psram"]["size"] = ESP.getPsramSize();
    doc["psram"]["free"] = ESP.getFreePsram();
    doc["psram"]["min_free"] = ESP.getMinFreePsram();
    doc["psram"]["max_alloc"] = ESP.getMaxAllocPsram();
    doc["psram"]["heap_caps_total"] = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
#endif

    // Flash Memory
    doc["flash"]["chip_size"] = ESP.getFlashChipSize();
    doc["flash"]["speed"] = ESP.getFlashChipSpeed();
    doc["flash"]["mode"] = ESP.getFlashChipMode();

    // Partition Information
    const esp_partition_t *running = esp_ota_get_running_partition();
    doc["partition"]["running_label"] = running->label;
    doc["partition"]["running_addr"] = running->address;
    doc["partition"]["running_size"] = running->size;
    doc["partition"]["app_size"] = ESP.getSketchSize();
    doc["partition"]["app_free"] = ESP.getFreeSketchSpace();

    // SD Card Information
    if (SD.begin())
    {
        doc["sd"]["available"] = true;
        doc["sd"]["total_bytes"] = SD.totalBytes();
        doc["sd"]["used_bytes"] = SD.usedBytes();
        doc["sd"]["total_mb"] = SD.totalBytes() / (1024 * 1024);
        doc["sd"]["used_mb"] = SD.usedBytes() / (1024 * 1024);
        doc["sd"]["type"] = SD.cardType();
    }
    else
    {
        doc["sd"]["available"] = false;
    }

    // Hardware Configuration (simplified)
    doc["hardware"]["adc_pin"] = 36; // Default ADC1 pin for ESP32

    // Application Statistics
    if (m_signal_processor_ptr)
    {
        doc["app_stats"]["noise_baseline"] = m_signal_processor_ptr->get_baseline();
        doc["app_stats"]["noise_current"] = m_signal_processor_ptr->get_current_value();
        doc["app_stats"]["noise_category"] = static_cast<int>(m_signal_processor_ptr->get_noise_category());
    }

    if (m_display_manager_ptr)
    {
        doc["app_stats"]["display_backlight"] = m_display_manager_ptr->get_backlight_active();
    }

    // Send response
    String output;
    serializeJson(doc, output);
    cors_headers();
    m_server.send(200, "application/json", output);
}