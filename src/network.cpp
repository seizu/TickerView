#include "globals.h"
#include "network.h"

HttpResult result;

/**
 * Performs an HTTP GET request to the specified URL
 *
 * @param url      The URL to fetch
 * @param result   Output struct containing HTTP status code, bytes read and payload
 * @param time_out Request timeout in milliseconds - default 3000
 * @return true if the request was successful (HTTP status 1xx-3xx), false on error or HTTP 4xx/5xx
 */
bool httpGet(const String &url, HttpResult &result, const uint16_t time_out) {
    result.code = -1;
    result.bytes_read = 0;
    result.payload[0] = '\0';

    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;
    http.setTimeout(time_out);

    if (http.begin(client, url)) {
        result.code = http.GET();

        if (result.code > 0) {
            String payload = http.getString();
            result.bytes_read = payload.length();
            if (result.bytes_read >= sizeof(result.payload)) {
                result.bytes_read = sizeof(result.payload) - 1;
            }
            memcpy(result.payload, payload.c_str(), result.bytes_read);
            result.payload[result.bytes_read] = '\0';
        }
        http.end();
    }
    return (result.code > 0 && result.code < 400);
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
                ntp_updated = tm.syncNTP();
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