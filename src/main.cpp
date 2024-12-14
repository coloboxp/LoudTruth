#include <Arduino.h>
#include "components/noise_monitor.hpp"
#include "components/wifi_manager.hpp"
#include "components/api_handler.hpp"

NoiseMonitor noise_monitor;

/**
 * @brief Setup function for the Arduino program.
 */
void setup()
{
  Serial.begin(115200);
  
  // Try to initialize WiFi and API, but continue if they fail
  if (wifi::WiFiManager::instance().init()) {
    ApiHandler::instance().begin();
  }
  
  // Initialize core noise monitoring functionality
  if (!noise_monitor.begin())
  {
    Serial.println("Failed to initialize noise monitor!");
  }
}

/**
 * @brief Main loop for the Arduino program.
 */
void loop()
{
  noise_monitor.update();
}