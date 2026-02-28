#include "display.h"
#include "globals.h"

void initDisplay(const char * hostip) {
    vspi.begin(SCLK_PIN, -1, DIN_PIN, -1);
    vspi.setFrequency(20000000);

    tft.begin();
    tft.fillScreen(BLACK);
    tft.setTextColor(RED);
    tft.setTextSize(1);
    tft.setCursor(0, 40);
    tft.printf("Init Display...\n\nWebPrefs IP:\n\n%s", hostip);
}

float getOldPrice(int asset_index) {
    // Bounds checking
    if (asset_index < 0 || asset_index >= NUM_ASSETS) {
        return 0.0f;
    }

    if (assets[asset_index].price_buffer == nullptr) {
        return 0.0f;
    }

    int oldest_index = buffer_full ? buffer_index : 0;

    // Bounds check for buffer index
    if (oldest_index < 0 || oldest_index >= buffer_size) {
        return assets[asset_index].current_price; // Fallback to current price
    }

    return assets[asset_index].price_buffer[oldest_index];
}

void displayAsset(const char* symbol, float price, float old_price, int digits, float change, int x_offset, int y_offset) {

    char text_buffer[40];
    char fmt[10];

    int16_t x1, y1;
    uint16_t w, h;

    tft.fillScreen(BLACK);
    snprintf(fmt, sizeof(fmt), "%%.%df", digits);

    // Symbol
    tft.setTextSize(3);
    tft.setTextColor(YELLOW_L);
    tft.getTextBounds(symbol, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(x_offset, 10 + y_offset);
    tft.print(symbol);

    // Store symbol width for positioning other elements
    uint16_t symbol_width = w;

    if(dc.show_percent) {
        // Epsilon comparison for float
        if(change < -0.001f) {
            tft.setTextColor(RED);
        } else if(change > 0.001f) {
            tft.setTextColor(GREEN);
        } else {
            tft.setTextColor(YELLOW_L);
        }
        // History price window info
        if(dc.show_hw) {
            sprintf(text_buffer, "%+.1f%% H%d", change, dc.history_window);
        } else {
            sprintf(text_buffer, "%+.1f%%", change);
        }
        tft.setTextSize(1);
        tft.setCursor(symbol_width + 5 + x_offset , 13 + y_offset);
        tft.print(text_buffer);
    }

    // History price
    if(dc.show_hp) {
        sprintf(text_buffer, fmt, old_price);
        tft.setTextColor(YELLOW_L);
        tft.setTextSize(1);
        tft.setCursor(symbol_width + 5 + x_offset , 23 + y_offset);
        tft.print(text_buffer);
    }

    // Price
    tft.setTextSize(2);
    tft.setTextColor(YELLOW);
    sprintf(text_buffer, fmt, price);
    tft.getTextBounds(text_buffer, 0, 0, &x1, &y1, &w, &h);
    tft.setCursor(x_offset, 40 + y_offset);
    tft.print(text_buffer);
}

void displayDateTime() {
    if(!dc.show_time) {
        return;
    }

    char time_str[20];
    int16_t x1, y1;
    uint16_t w, h;

    time_t now = time(nullptr);
    struct tm* timeinfo = localtime(&now);
    strftime(time_str, sizeof(time_str), "%d.%b %H:%M:%S", timeinfo);

    tft.setTextSize(1);
    tft.getTextBounds(time_str, 0, 0, &x1, &y1, &w, &h);

    // Clear only the time display area (x, y, width, height)
    tft.fillRect(dc.x_offset, 75 + y_offset, w, h, BLACK);

    // Draw updated time
    tft.setTextColor(LIGHTBLUE);
    tft.setCursor(dc.x_offset, 75 + y_offset);
    tft.print(time_str);
}
