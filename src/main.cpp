#include <Arduino.h>
#include <U8g2lib.h>
#include <SD.h>
#include "config.h"

// Statistics structure definition needs to come before function declarations
struct Statistics
{
  int min = 1023;
  int max = 0;
  float avg = 0;
  int samples = 0;
};

// Forward declarations
String getNoiseCategoryString(float value);
void updateEMA();
void updateStatistics(Statistics &stats, float value);
void drawStats();
void setupSD();
void logToSD();

// Variables for EMA calculation
float emaValue = 0;
float baselineEMA = 0;

// Statistics variables
Statistics dailyStats;
Statistics fifteenMinStats;
Statistics oneMinStats;

// Circular buffer for plotting
int plotBuffer[display_config::plot::PLOT_POINTS];
int plotIndex = 0;

// Initialize U8G2 display with ST7565 driver
U8G2_ST7565_ERC12864_ALT_F_4W_HW_SPI u8g2(
    U8G2_R0,
    pins::display::CS,   // CS
    pins::display::DC,   // DC
    pins::display::RESET // RESET
);

void setupSD()
{
  if (!SD.begin(pins::sd::CS))
  {
    Serial.println("SD card initialization failed!");
    return;
  }
  Serial.println("SD card initialized.");

  // Create headers if file doesn't exist
  if (!SD.exists("noise_log.csv"))
  {
    File dataFile = SD.open("noise_log.csv", FILE_WRITE);
    if (dataFile)
    {
      dataFile.println("timestamp,noise,baseline,category,1min_avg,15min_avg");
      dataFile.close();
    }
  }
}

void logToSD()
{
  File dataFile = SD.open("noise_log.csv", FILE_WRITE);
  if (dataFile)
  {
    // Format: timestamp,current_noise,baseline,category,1min_avg,15min_avg
    dataFile.print(millis());
    dataFile.print(",");
    dataFile.print(emaValue);
    dataFile.print(",");
    dataFile.print(baselineEMA);
    dataFile.print(",");
    dataFile.print(getNoiseCategoryString(emaValue));
    dataFile.print(",");
    dataFile.print(oneMinStats.avg);
    dataFile.print(",");
    dataFile.println(fifteenMinStats.avg);
    dataFile.close();
  }
}

void updateEMA()
{
  // Read multiple samples and average them
  long sum = 0;
  for (int i = 0; i < 32; i++)
  {
    sum += analogRead(pins::analog::SOUND);
  }
  int rawValue = sum >> 5; // Divide by 32

  // Calculate fast EMA for current noise
  emaValue = (signal_processing::EMA_ALPHA * rawValue) + ((1.0 - signal_processing::EMA_ALPHA) * emaValue);

  // Update baseline (very slow EMA)
  if (baselineEMA == 0)
    baselineEMA = emaValue; // Initialize baseline
  baselineEMA = (signal_processing::BASELINE_ALPHA * emaValue) + ((1.0 - signal_processing::BASELINE_ALPHA) * baselineEMA);
}

String getNoiseCategoryString(float value)
{
  float ratio = value / baselineEMA;
  if (ratio < signal_processing::thresholds::NOISE_REGULAR)
    return "OK";
  if (ratio < signal_processing::thresholds::NOISE_HIGH)
    return "REGULAR";
  if (ratio < signal_processing::thresholds::NOISE_TOXIC)
    return "HIGH";
  return "TOXIC";
}

void updateStatistics(Statistics &stats, float value)
{
  stats.min = min(stats.min, (int)value);
  stats.max = max(stats.max, (int)value);
  stats.avg = ((stats.avg * stats.samples) + value) / (stats.samples + 1);
  stats.samples++;
}

void drawStats()
{
  u8g2.clearBuffer();

  // Draw noise level and category
  u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.drawStr(0, 10, ("Noise: " + String((int)emaValue)).c_str());
  u8g2.drawStr(64, 10, getNoiseCategoryString(emaValue).c_str());

  // Draw statistics
  u8g2.drawStr(0, 20, ("1m:  " + String((int)oneMinStats.avg) + " [" + String(oneMinStats.min) + "-" + String(oneMinStats.max) + "]").c_str());
  u8g2.drawStr(0, 30, ("15m: " + String((int)fifteenMinStats.avg) + " [" + String(fifteenMinStats.min) + "-" + String(fifteenMinStats.max) + "]").c_str());

  // Draw plot
  for (int i = 0; i < display_config::plot::PLOT_POINTS - 1; i++)
  {
    int x1 = i * 2;
    int x2 = (i + 1) * 2;
    int y1 = display_config::plot::PLOT_BASELINE_Y_POSITION - map(plotBuffer[i], 0, 1023, 0, display_config::plot::PLOT_HEIGHT);
    int y2 = display_config::plot::PLOT_BASELINE_Y_POSITION - map(plotBuffer[(i + 1)], 0, 1023, 0, display_config::plot::PLOT_HEIGHT);
    u8g2.drawLine(x1, y1, x2, y2);
  }

  u8g2.sendBuffer();
}

void setup()
{
  Serial.begin(115200);
  setupSD();
  u8g2.begin();
  pinMode(pins::display::BACKLIGHT, OUTPUT);
  digitalWrite(pins::display::BACKLIGHT, HIGH);
}

void loop()
{
  static unsigned long lastSampleTime = 0;
  static unsigned long lastDisplayTime = 0;
  static unsigned long lastLogTime = 0;

  unsigned long currentTime = millis();

  // Update EMA and collect data
  if (currentTime - lastSampleTime >= intervals::SAMPLE_INTERVAL)
  {
    updateEMA();

    // Update plot buffer
    plotBuffer[plotIndex] = (int)emaValue;
    plotIndex = (plotIndex + 1) % display_config::plot::PLOT_POINTS;

    // Update statistics
    updateStatistics(oneMinStats, emaValue);
    updateStatistics(fifteenMinStats, emaValue);
    updateStatistics(dailyStats, emaValue);

    lastSampleTime = currentTime;
  }

  // Update display
  if (currentTime - lastDisplayTime >= intervals::DISPLAY_INTERVAL)
  {
    drawStats();
    lastDisplayTime = currentTime;
  }

  // Log to SD card
  if (currentTime - lastLogTime >= intervals::LOG_INTERVAL)
  {
    logToSD();
    lastLogTime = currentTime;
  }
}