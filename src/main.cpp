#include <Arduino.h>
#include "globals.h"
#include "network.h"
#include "storage.h"
#include "display.h"
#include "crypto.h"

SPIClass vspi = SPIClass(VSPI);
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &vspi, CS_PIN, DC_PIN, RST_PIN);

// Array of asset data
AssetData assets[NUM_ASSETS];

int buffer_size = 0;
int buffer_index = 0;
bool buffer_full = false;

// Timer
ulong last_update = 0;
ulong last_rotation = 0;
ulong last_time_update = 0;

#define MAX_Y 45
#define MOVE_Y 5

int y_offset = 0;
int bounce_direction_y = MOVE_Y;

// Current asset
int current_asset = 0;

void setup() {
    Serial.begin(115200);

    initWebPrefs();
    initWifi();
    cbNTPConfigUpdate();
    initCrypto();
    initDisplay(WiFi.localIP().toString().c_str());

    last_update = TIMENOW;
    last_rotation = TIMENOW;
    last_time_update = TIMENOW;
}

void loop() {
    yield();

    handleWiFi();

    // Get current time once per loop iteration
    ulong current_millis = TIMENOW;

    // Check if price update is needed
    if ((ulong)(current_millis - last_update) >= (ulong)dc.price_update * 60000UL) {
        LOG_SDEBUG("Starting price update cycle...");
        updatePrices();

        // Calculate percentage change on each update
        calculateChanges();

        last_update = current_millis;
        LOG_SDEBUG("Price update complete");
    }

    // Asset rotation
    if ((ulong)(current_millis - last_rotation) >= (ulong)dc.display_time * 1000UL) {
        current_asset = (current_asset + 1) % NUM_ASSETS;
        last_rotation = current_millis;

        // Y-offset bouncing (anti-burn-in protection)
        y_offset += bounce_direction_y;
        if (y_offset >= MAX_Y) {
            bounce_direction_y = -MOVE_Y;
        } else if (y_offset <= 0) {
            bounce_direction_y = MOVE_Y;
        }

        // Display current asset
        AssetData& asset = assets[current_asset];
        displayAsset(asset.asset_name, asset.current_price, getOldPrice(current_asset),
                     asset.digits, asset.change_percent, dc.x_offset, y_offset);

        displayDateTime();
        last_time_update = current_millis;
    }

    // Update time display every second
    if ((ulong)(current_millis - last_time_update) >= 1000UL) {
        displayDateTime();
        last_time_update = current_millis;
    }
    // update NTP
    updateNTP();
    delay(100);
}