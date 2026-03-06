#include "globals.h"
#include "display.h"
#include "network.h"

float getBinancePrice(const char* symbol) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_SDEBUG("WiFi not connected");
        return 0.0f;
    }

    char url[128];
    snprintf(url, sizeof(url), "https://fapi.binance.com/fapi/v1/premiumIndex?symbol=%s", symbol);

    float price = 0.0f;
    if(httpGet(url, result)) {

        JsonDocument doc;
        DeserializationError error = deserializeJson(doc, result.payload);

        if (error) {
            LOG_SERROR("JSON parse failed for %s: %s", symbol, error.c_str());
            return 0.0f;
        }

        // Validate that indexPrice field exists and is valid
        if (!doc["indexPrice"].isNull()) {
            price = doc["indexPrice"].as<float>();

            // Validate the price is a real number
            if (isnan(price) || isinf(price) || price < 0.0f) {
                LOG_SERROR("Invalid price for %s: %f", symbol, price);
                price = 0.0f;
            }
        } else {
            LOG_SERROR("indexPrice field missing for %s", symbol);
        }
    } else {
        LOG_SERROR("HTTP error for %s: %d", symbol, result.code);
    }

    return price;
}

void updatePrices() {
    LOG_SDEBUG("Fetching prices... Free heap: %d bytes", ESP.getFreeHeap());

    // Fetch all prices and store in buffers
    for (int i = 0; i < NUM_ASSETS; i++) {
        yield(); // Feed watchdog before each request

        float new_price = getBinancePrice(assets[i].symbol);

        // Only update if we got a valid price (not 0 from error)
        if (new_price > 0.0f) {
            assets[i].current_price = new_price;
            assets[i].price_buffer[buffer_index] = new_price;
        } else {
            LOG_SDEBUG("Skipping invalid price for %s, keeping previous", assets[i].symbol);
            // Keep the previous price in the buffer
            assets[i].price_buffer[buffer_index] = assets[i].current_price;
        }

        if (i < NUM_ASSETS - 1) {
            delay(100); // Small delay between requests (also feeds watchdog)
        }
    }

    LOG_SDEBUG("Prices updated! Free heap: %d bytes", ESP.getFreeHeap());

    // Increment circular buffer index
    buffer_index = (buffer_index + 1) % buffer_size;

    // Buffer is full when index reaches 0 again
    if (buffer_index == 0 && !buffer_full) {
        buffer_full = true;
        LOG_SINFO("Buffer full - Rolling window active!");
    }
}

float getOldPrice(int asset_index) {
    if (asset_index < 0 || asset_index >= NUM_ASSETS) {
        return 0.0f;
    }

    int oldest_index = buffer_full ? buffer_index : 0;
    return assets[asset_index].price_buffer[oldest_index];
}

void calculateChanges() {
    int oldest_index = buffer_full ? buffer_index : 0;

    for (int i = 0; i < NUM_ASSETS; i++) {
        float old_price = assets[i].price_buffer[oldest_index];
        if (old_price > 0.0f) {
            assets[i].change_percent = ((assets[i].current_price - old_price) / old_price) * 100.0f;
        } else {
            assets[i].change_percent = 0.0f;
        }
    }
}

void initCrypto() {

    // (WebPrefs already validates history_window: 1-24, price_update: 1-60)
    buffer_size = (dc.history_window * 60) / dc.price_update + 1;
    
    LOG_SINFO("Buffer size: %d entries (%d bytes per asset, %d total)",
              buffer_size,
              buffer_size * sizeof(float),
              buffer_size * sizeof(float) * NUM_ASSETS);
    LOG_SINFO("Free heap before allocation: %d bytes", ESP.getFreeHeap());

    assets[0] = {nullptr, 0.0f, 0.0f, dc.symbol1, dc.asset1, dc.digits1};
    assets[1] = {nullptr, 0.0f, 0.0f, dc.symbol2, dc.asset2, dc.digits2};
    assets[2] = {nullptr, 0.0f, 0.0f, dc.symbol3, dc.asset3, dc.digits3};
    assets[3] = {nullptr, 0.0f, 0.0f, dc.symbol4, dc.asset4, dc.digits4};

    // Allocate and initialize history buffers
    for (int i = 0; i < NUM_ASSETS; i++) {
        assets[i].price_buffer = new float[buffer_size]();
    }

    LOG_SINFO("Free heap after allocation: %d bytes", ESP.getFreeHeap());
    
    // Fetch initial data
    updatePrices();
    // Calculate initial percentage changes
    calculateChanges();     
}
