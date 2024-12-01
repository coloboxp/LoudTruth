#include <Arduino.h>
#include "components/noise_monitor.hpp"

NoiseMonitor noise_monitor;

void setup()
{
  Serial.begin(115200);
  if (!noise_monitor.begin())
  {
    Serial.println("Failed to initialize noise monitor!");
  }
}

void loop()
{
  noise_monitor.update();
}