#ifndef APSSID
#define APSSID "Ecto-1_ap"
#define APPSK  "12345678"
#endif

#include "SubBoard.hpp"
#include "WebServer.hpp"

SubBoard *board;
void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  board = new SubBoard();

  DEBUG_ESP_PORT.begin(115200);
  DEBUG_ESP_PORT.printf("\n\nSdk version: %s\n", ESP.getSdkVersion());
  DEBUG_ESP_PORT.printf("Core Version: %s\n", ESP.getCoreVersion().c_str());
  DEBUG_ESP_PORT.printf("Boot Version: %u\n", ESP.getBootVersion());
  DEBUG_ESP_PORT.printf("Boot Mode: %u\n", ESP.getBootMode());
  DEBUG_ESP_PORT.printf("CPU Frequency: %u MHz\n", ESP.getCpuFreqMHz());
  DEBUG_ESP_PORT.printf("Reset reason: %s\n", ESP.getResetReason().c_str());

  setupWifi();
}
void loop() {
  wifiLoop();
}
