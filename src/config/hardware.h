#ifndef HARDWARE_H
#define HARDWARE_H

// Pin definitions
#define MCU "ESP32"
#define RESTORE_PIN 33
#define SCLK_PIN  18
#define DIN_PIN   23
#define DC_PIN    16
#define CS_PIN    5
#define RST_PIN   17
#define TIMENOW millis()

// uncomment it to use default instead of flash rom settings
#define READCONFIG

// NTP interval in minutes
#define NTP_UPDATE_INTERVAL 720

#endif // HARDWARE_H
