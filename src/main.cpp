#include <Arduino.h>
#include "globals.h"
#include "network.h"
#include "storage.h"
#include "display.h"

SPIClass vspi = SPIClass(VSPI);
Adafruit_SSD1351 tft = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &vspi, CS_PIN, DC_PIN, RST_PIN);

// Array of asset data
AssetData assets[NUM_ASSETS];

int buffer_size = 0;
int buffer_index = 0;
bool buffer_full = false;

// Timer
unsigned long last_update = 0;
unsigned long last_rotation = 0;
unsigned long last_time_update = 0;

#define MAX_Y 45
#define MOVE_Y 5

int y_offset = 0;
int bounce_direction_y = MOVE_Y;

// Current asset
int current_asset = 0;

void updatePrices() {
    LOG_SDEBUG("Fetching prices...");

    // Fetch all prices and store in buffers
    for (int i = 0; i < NUM_ASSETS; i++) {
        assets[i].current_price = getPrice(assets[i].symbol);
        assets[i].price_buffer[buffer_index] = assets[i].current_price;

        if (i < NUM_ASSETS - 1) {
            delay(100); // Small delay between requests
        }
    }

    LOG_SDEBUG("Prices updated!");

    // Increment index
    buffer_index = (buffer_index + 1) % buffer_size;

    // Buffer is full when index reaches 0 again
    if (buffer_index == 0 && !buffer_full) {
        buffer_full = true;
        LOG_SINFO("Buffer full - Rolling window active!");
    }
}

void calculateChanges() {
    // Oldest index in buffer (calculated once)
    int oldest_index = buffer_full ? buffer_index : 0;

    // Calculate percentage change for all assets
    for (int i = 0; i < NUM_ASSETS; i++) {
        float old_price = assets[i].price_buffer[oldest_index];
        if (old_price > 0.0001f) { // Epsilon comparison
            assets[i].change_percent = ((assets[i].current_price - old_price) / old_price) * 100.0f;
        } else {
            assets[i].change_percent = 0.0f;
        }
    }
}

void setup() {
    Serial.begin(115200);

    initWebPrefs();
    initWifi();

    buffer_size = (dc.history_window * 60) / dc.price_update;

    LOG_SINFO("Buffer size: %d entries", buffer_size);

    // Initialize asset data structure
    assets[0] = {nullptr, 0.0f, 0.0f, dc.symbol1, dc.asset1, dc.digits1};
    assets[1] = {nullptr, 0.0f, 0.0f, dc.symbol2, dc.asset2, dc.digits2};
    assets[2] = {nullptr, 0.0f, 0.0f, dc.symbol3, dc.asset3, dc.digits3};
    assets[3] = {nullptr, 0.0f, 0.0f, dc.symbol4, dc.asset4, dc.digits4};

    // Allocate and initialize history buffers
    for (int i = 0; i < NUM_ASSETS; i++) {
        assets[i].price_buffer = new float[buffer_size];
        if (assets[i].price_buffer == nullptr) {
            LOG_SERROR("Failed to allocate memory for asset %d", i);
            // Fatal error - halt execution
            while(1) {
                delay(1000);
            }
        }
        memset(assets[i].price_buffer, 0, buffer_size * sizeof(float));
    }

    // Initialize display
    initDisplay(WiFi.localIP().toString().c_str());
    delay(1000);

    // Initialize NTP
    cbNTPConfigUpdate();

    // Fetch initial data
    updatePrices();

    // Calculate initial percentage changes
    calculateChanges();

    last_update = millis();
    last_rotation = millis();
    last_time_update = millis();

    // Display first asset
    displayAsset(assets[0].asset_name, assets[0].current_price, getOldPrice(0),
                 assets[0].digits, assets[0].change_percent, dc.x_offset, y_offset);

    // Display initial time
    displayDateTime();
}

void loop() {
    handleWiFi();

    // Get current time once per loop iteration
    unsigned long current_millis = millis();

    // Check if price update is needed (overflow-safe)
    if ((unsigned long)(current_millis - last_update) >= (unsigned long)dc.price_update * 60000UL) {
        updatePrices();

        // Calculate percentage change on each update
        calculateChanges();

        last_update = current_millis;
    }

    // Asset rotation (overflow-safe)
    if ((unsigned long)(current_millis - last_rotation) >= (unsigned long)dc.display_time * 1000UL) {
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

        // Update time immediately after asset change
        displayDateTime();
        last_time_update = current_millis;
    }

    // Update time display every second (overflow-safe)
    if ((unsigned long)(current_millis - last_time_update) >= 1000UL) {
        displayDateTime();
        last_time_update = current_millis;
    }

    // update NTP
    updateNTP();

    delay(100);
}