#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>

// Display dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Colors
#define BLACK      0x0000
#define WHITE      0xFFFF
#define YELLOW     0xFFE0
#define YELLOW_L   0xFFE8
#define GREEN      0x07E0
#define RED        0xF800
#define ORANGE     0xFD20
#define BLUE       0x001F
#define LIGHTBLUE  0x867D

// Asset data structure
struct AssetData {
    float* price_buffer;
    float current_price;
    float change_percent;
    const char* symbol;
    const char* asset_name;
    int digits;
};

// Number of assets
#define NUM_ASSETS 4

// External references
extern SPIClass vspi;
extern Adafruit_SSD1351 tft;
extern AssetData assets[NUM_ASSETS];
extern int buffer_index;
extern bool buffer_full;
extern int y_offset;

// Display functions
void initDisplay(const char * hostip);
void displayAsset(const char* symbol, float price, float old_price, int digits, float change, int x_offset, int y_offset);
void displayDateTime();
float getOldPrice(int asset_index);

#endif // DISPLAY_H
