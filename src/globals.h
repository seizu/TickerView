#ifndef GLOBALS_H
#define GLOBALS_H

#define TICKERVIEW_VERSION "1.1"

#include <Arduino.h>
#include "secrets.h"
#include "config/hardware.h"
#include <EEPROM.h>
#include <ESPAsyncWebServer.h>

#if defined(ESP8266)
  #include <ESPAsyncTCP.h>
  #include <ESP8266HTTPClient.h>
  #include <WiFiClientSecure.h>
#elif defined(ESP32)
  #include <AsyncTCP.h>
  #include <HTTPClient.h>
  #include <WiFiClientSecure.h>
#endif

#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

#include "SimpleWifi.h"
#include "WebPrefs.h"
#include "config/WebPrefsConfig.h"
#include "RealTimeClock.h"

// --- global instances ---
extern DeviceConfig dc;
extern RealTimeClock rtc;
extern SimpleWifi sw;
extern WebPrefs wp;
extern ulong ntp_update_event;
extern bool ntp_updated;

#endif // GLOBALS_H
