#include "api_handler.hpp"
#include "wifi_manager.hpp"
#include "esp_log.h"
#include <ArduinoJson.h>

void ApiHandler::begin()
{
    ESP_LOGI(TAG, "Initializing API handler");

    // Initialize HTTP client with SSL
    m_secure_client.setInsecure(); // For ThingSpeak we can use insecure mode

    // Verify channel exists and API keys are set
    if (!config::thingspeak::NOISE_CHANNEL_ID ||
        !config::thingspeak::NOISE_API_KEY)
    {
        ESP_LOGE(TAG, "Missing ThingSpeak configuration");
        m_available = false;
        return;
    }

    // Verify the keys are not the default placeholder values
    if (strcmp(config::thingspeak::NOISE_API_KEY, "your_thingspeak_noise_api_key") == 0 ||
        strcmp(config::thingspeak::NOISE_CHANNEL_ID, "your_noise_channel_id") == 0)
    {
        ESP_LOGE(TAG, "ThingSpeak configuration contains default values");
        m_available = false;
        return;
    }

    // If we get here, configuration is valid
    m_available = true;
    ESP_LOGI(TAG, "API handler initialization %s", m_available ? "successful" : "failed");
}

bool ApiHandler::ensure_channel_exists()
{
    ESP_LOGI(TAG, "Verifying ThingSpeak channel configuration");

    if (!m_available)
    {
        m_last_error = "Handler not properly initialized";
        ESP_LOGE(TAG, "%s", m_last_error.c_str());
        return false;
    }

    // Additional verification could be added here
    // For example, making a test request to verify the channel exists

    return true;
}

bool ApiHandler::send_noise_data(const JsonDocument &doc)
{
    if (!m_available)
    {
        ESP_LOGW(TAG, "Handler not available, skipping");
        return false;
    }

    // Check rate limiting - ThingSpeak requires 15 seconds between updates
    unsigned long now = millis();
    if (now - m_last_request < config::thingspeak::UPDATE_INTERVAL_MS)
    {
        ESP_LOGD(TAG, "Rate limited, skipping request. Next update in %lu ms",
                 config::thingspeak::UPDATE_INTERVAL_MS - (now - m_last_request));
        return false;
    }
    m_last_request = now;

    if (!wifi::WiFiManager::instance().ensure_connected())
    {
        m_last_error = "WiFi connection lost";
        ESP_LOGE(TAG, "%s", m_last_error.c_str());
        return false;
    }

    // Create bulk update payload following ThingSpeak format
    JsonDocument payload_doc;

    // Use the NOISE_API_KEY for writing, not the USER_API_KEY
    payload_doc["write_api_key"] = config::thingspeak::NOISE_API_KEY;
    JsonArray updates = payload_doc["updates"].to<JsonArray>();

    // Add the update object
    JsonObject update = updates.add<JsonObject>();

    // Get current timestamp
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
    {
        ESP_LOGE(TAG, "Failed to obtain time");
        return false;
    }

    char timestamp[30];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &timeinfo);
    update["created_at"] = timestamp;

    // Copy fields from input document
    for (JsonPairConst kv : doc.as<JsonObjectConst>())
    {
        update[kv.key()] = kv.value();
    }

    String payload;
    serializeJson(payload_doc, payload);
    ESP_LOGD(TAG, "Sending payload: %s", payload.c_str());

    // Construct URL for bulk update
    String url = String(config::thingspeak::ENDPOINT) + "/channels/" +
                 String(config::thingspeak::NOISE_CHANNEL_ID) +
                 "/bulk_update.json";

    ESP_LOGD(TAG, "Sending to URL: %s", url.c_str());

    // Ensure we end any previous connection
    m_http_client.end();

    // Configure secure client
    m_secure_client.setInsecure();

    // Begin new connection with retry logic
    int retries = 3;
    while (retries > 0)
    {
        if (m_http_client.begin(m_secure_client, url))
        {
            m_http_client.addHeader("Content-Type", "application/json");

            int httpCode = m_http_client.POST(payload);

            if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_ACCEPTED)
            {
                String response = m_http_client.getString();
                ESP_LOGD(TAG, "ThingSpeak Response: %s", response.c_str());
                m_http_client.end();
                return true;
            }

            if (httpCode == 429)
            { // Too Many Requests
                ESP_LOGW(TAG, "Rate limit exceeded, waiting for 15 seconds...");
                m_http_client.end();
                delay(15000); // Wait full 15 seconds before retry
                retries--;
                continue;
            }

            if (httpCode == -11)
            { // Timeout
                ESP_LOGW(TAG, "HTTP POST timeout, retrying... (%d attempts left)", retries - 1);
                retries--;
                m_http_client.end();
                delay(1000);
                continue;
            }

            ESP_LOGE(TAG, "HTTP POST failed, code: %d", httpCode);
            break;
        }

        ESP_LOGE(TAG, "Failed to begin HTTP client");
        retries--;
        delay(1000);
    }

    m_http_client.end();
    delay(100);
    return false;
}