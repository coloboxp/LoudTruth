#include <Arduino.h>
#include "components/noise_monitor.hpp"

NoiseMonitor noise_monitor;

/**
 * @brief Setup function for the Arduino program.
 */
void setup()
{
  Serial.begin(115200);
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