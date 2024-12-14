#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <map>
#include <string>
#include "config/config.h"
#include <ArduinoJson.h>

class ApiHandler
{
public:
    static ApiHandler &instance()
    {
        static ApiHandler instance;
        return instance;
    }

    void begin();
    bool send_noise_data(const JsonDocument &doc);
    bool is_available() const { return m_available; }

private:
    ApiHandler() = default;

    struct ChannelInfo
    {
        std::string write_api_key;
        std::string read_api_key;
    };

    HTTPClient m_http_client;
    WiFiClientSecure m_secure_client;
    unsigned long m_last_request{0};
    std::string m_last_error;
    bool m_available{false};

    static constexpr char const *TAG = "ApiHandler";

    bool ensure_channel_exists();
};