#include "globals.h"
#include "network.h"


float getPrice(const char* symbol) {
    if (WiFi.status() != WL_CONNECTED) {
        LOG_SDEBUG("WiFi not connected");
        return 0;
    }

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    char url[128];
    snprintf(url, sizeof(url), "https://fapi.binance.com/fapi/v1/premiumIndex?symbol=%s", symbol);

    http.begin(client, url);
    http.setTimeout(5000); // 5 second timeout
    int http_code = http.GET();

    float price = 0.0f;
    if (http_code == 200) {
        String payload = http.getString();

        // Specify JsonDocument size (Binance API response ~200 bytes)
        JsonDocument doc;

        // Parse JSON with error checking
        DeserializationError error = deserializeJson(doc, payload);

        if (error) {
            LOG_SERROR("JSON parse failed for %s: %s", symbol, error.c_str());
            http.end();
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
        LOG_SERROR("HTTP error for %s: %d", symbol, http_code);
    }

    http.end();
    return price;
}

/**
 * Manages WiFi connectivity and Web UI availability
 * - Periodically checks and enforces the desired WiFi mode
 * - Disables WiFi after inactivity if configured (`disable_wifi` and `web_idle_timeout`)
 * - Starts Web UI when WiFi is established (STA or AP mode)
 * - Switches to AP fallback mode if STA connection fails after a defined number of attempts
 */
void handleWiFi() {
    auto wifi_status = sw.ensureWifiMode(dc.wifi_check_sec * 1000); // non-blocking call, default check status every 5000ms

    // prevent disable wifi during status PENDING
    if (wifi_status == SimpleWifi::PENDING) {
        wp.resetIdleTime();
    }
/*
    // disable WiFi when 'disable_wifi' is true and the web interface has timed out due to inactivity
    if (dc.disable_wifi == true && WiFi.getMode() != WIFI_OFF && wp.getIdleTime() >= (dc.web_idle_timeout * 60 * 1000)) {
        // Stop Web Interface
        wp.stop();
        sw.disableWifi();
    }
*/
    if (wifi_status == SimpleWifi::AP_ESTABLISHED || wifi_status == SimpleWifi::STA_ESTABLISHED) {
        // Start Web Interface
        wp.start();
        // check for AP fallback condition
    } else if (sw.getMode() == WIFI_STA && dc.ap_fallback > 0 && sw.getAttempts() >= dc.ap_fallback) {
        LOG_SDEBUG("AP Fallback activated");
        wp.stop();
        sw.setMode(WIFI_AP);
    }
}

/**
 * Update system time
**/
void updateNTP() {
    if(dc.ntp_enabled) {
        if(((TIMENOW - ntp_update_event) > NTP_UPDATE_INTERVAL * 1000 * 60) || ntp_update_event == 0) {
            if(WiFi.isConnected()) {
                ntp_updated = rtc.syncNTP();
                if(ntp_updated) {
                    ntp_update_event = TIMENOW;
                }
            } else if(dc.ap_only == false && sw.getStatus() != SimpleWifi::PENDING) {
                sw.setMode(WIFI_STA);
                wp.getIdleTime();
            }
        }
    }
}

/**
 * Init WIFI
**/
void initWifi() {    
    LOG_SINFO("--- Init WIFI ---");
    // prepare Station
    sw.configStation(
        dc.wifi_ssid,
        dc.sta_wifi_passwd,
        0,
        dc.staticip_enabled,
        dc.ip_address,
        dc.gateway_address,
        dc.subnetmask,
        dc.dns1_address,
        dc.dns2_address);

    // check restore mode (RESTORE_PIN HIGH = restore)
    bool restore_mode = digitalRead(RESTORE_PIN) == HIGH;

    // prepare Access Point
    sw.configAccessPoint(nullptr, restore_mode ? AP_PASSWD : dc.ap_wifi_passwd, dc.ap_channel);

    // set wifi operation mode
    if (dc.ap_only || restore_mode) {
        sw.setMode(WIFI_AP);
    } else {
        sw.setMode(WIFI_STA);
    }

    // if we are in restore mode disable UI password
    if (restore_mode) {
        wp.setAuthentication(false);
    } else {
        wp.setAuthentication(dc.web_auth, dc.web_user, dc.web_passwd);
    }
    
    // activate wifi
    sw.ensureWifiMode(1000, 5); // try enable wifi, blocking mode: 1000ms delay, 5 retries
}