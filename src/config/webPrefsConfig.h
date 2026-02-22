#ifndef WEBPREFSCONFIG_H
#define WEBPREFSCONFIG_H

#include "webPrefs.h"
#include "webPrefsMacros.h"

void cbNTPConfigUpdate();

#define VAR_NAME(variable) #variable

// EEPROM prefs data
// --------------------------------------------------------------
struct DeviceConfig {
    // ===== Framework: Storage Header =====
    char header[4];
    uint16_t check_sum;

    // ===== Display =============
    char symbol1[17];
    char symbol2[17];
    char symbol3[17];
    char symbol4[17];
    char asset1[7];
    char asset2[7];
    char asset3[7];
    char asset4[7];
    uint16_t digits1;
    uint16_t digits2;
    uint16_t digits3;
    uint16_t digits4;

    uint16_t history_window;
    uint16_t price_update;
    uint16_t display_time;
    uint16_t x_offset;
    bool show_percent;
    bool show_hw;
    bool show_hp;
    bool show_time;

    // ===== Framework: WiFi =====
    char wifi_ssid[33];
    char sta_wifi_passwd[64];
    bool staticip_enabled;
    char ip_address[16];
    char subnetmask[16];
    char gateway_address[16];
    char dns1_address[16];
    char dns2_address[16];
    bool ap_only;
    char ap_wifi_passwd[64];
    uint16_t ap_channel;
    uint16_t ap_fallback;    // AP fall back mode (after retries) / 0 = disabled
    uint16_t wifi_check_sec; // wifi check interval in seconds

    // ===== Framework: WebPrefs =====
    bool web_auth;
    char web_user[17];
    char web_passwd[64];
    uint16_t web_idle_timeout;

    // ===== Framework: NTP/Clock =====
    bool ntp_enabled;
    char ntp_server[128];
    char tz_string[50];
    int32_t gmt_offset;
    uint16_t daylight_offset;

    char __STORAGE_END__;
    // <-- values not stored into flash mem -->
    bool sw_state;             // current switch state, true = on / false = off
    char info_text[256];       // device information shown in WebPrefs
    char date_time[20];
};
// --------------------------------------------------------------

// default prefs
const std::vector<WebPrefs::input_field> input_fields = {
// ===== Display =============
    FIELD_STRING(   symbol1,          "BTCUSDT",            3,        nullptr),
    FIELD_STRING(   symbol2,          "ETHUSDT",            3,        nullptr),
    FIELD_STRING(   symbol3,          "XAUUSDT",            3,        nullptr),
    FIELD_STRING(   symbol4,          "XAGUSDT",            3,        nullptr), 

    FIELD_STRING(   asset1,          "BTC",                 1,        nullptr),
    FIELD_STRING(   asset2,          "ETH",                 1,        nullptr),
    FIELD_STRING(   asset3,          "XAU",                 1,        nullptr),
    FIELD_STRING(   asset4,          "XAG",                 1,        nullptr),

    FIELD_UINT16(   digits1,          "2",                  1, 7,     nullptr),
    FIELD_UINT16(   digits2,          "2",                  1, 7,     nullptr),
    FIELD_UINT16(   digits3,          "2",                  1, 7,     nullptr),
    FIELD_UINT16(   digits4,          "2",                  1, 7,     nullptr),


    FIELD_UINT16(   price_update,     "1",                  1, 60,    nullptr),
    FIELD_UINT16(   display_time,     "5",                  1, 60,    nullptr),
    FIELD_UINT16(   history_window,   "12",                 1, 24,    nullptr),
    FIELD_CHECKBOX( show_percent,     "on",                           nullptr),
    FIELD_CHECKBOX( show_hw,          "on",                           nullptr),
    FIELD_CHECKBOX( show_hp,          "on",                           nullptr),
    FIELD_CHECKBOX( show_time,        "on",                           nullptr),  
    FIELD_UINT16(   x_offset,         "5",                  0, 10,    nullptr),

// ===== Framework: WiFi =====
    FIELD_STRING(   wifi_ssid,        WIFI_SSID,            1,        nullptr),
    FIELD_PASSWORD( sta_wifi_passwd,  WIFI_PASSWD ,         8,        nullptr),
    FIELD_CHECKBOX( ap_only,          "on",                           nullptr),
    FIELD_PASSWORD( ap_wifi_passwd,   AP_PASSWD,            8,        nullptr),
    FIELD_UINT16(   ap_channel,       "1",                  1, 13,    nullptr),
    FIELD_UINT16(   ap_fallback,      "34560",              0, 65535, nullptr),
    FIELD_UINT16(   wifi_check_sec,   "5",                  5, 65535, nullptr),

    FIELD_CHECKBOX( staticip_enabled, "off",                          nullptr),
    FIELD_STRING(   ip_address,       "192.168.0.2",        0,        nullptr),
    FIELD_STRING(   subnetmask,       "255.255.255.0",      0,        nullptr),
    FIELD_STRING(   gateway_address,  "192.168.0.1",        0,        nullptr),
    FIELD_STRING(   dns1_address,     "192.168.0.1",        0,        nullptr),
    FIELD_STRING(   dns2_address,     "",                   0,        nullptr),

    //FIELD_CHECKBOX( disable_wifi,     "off",                          nullptr),
    //FIELD_UINT16(   wifi_on_toggle,   "5",                  0, 10,    nullptr),

// ===== Framework: WebPrefs =====
    FIELD_CHECKBOX( web_auth,         "off",                          nullptr),
    FIELD_STRING(   web_user,         WEB_USER,             3,        nullptr),
    FIELD_PASSWORD( web_passwd,       WEB_PASSWD,           0,        nullptr),
    FIELD_UINT16(   web_idle_timeout, "1",                  1, 60,    nullptr),

// ===== Framework: NTP/Clock =====
    FIELD_CHECKBOX( ntp_enabled,       "off",                         nullptr),
    FIELD_STRING(   ntp_server,        "ts1.univie.ac.at",     0,     cbNTPConfigUpdate),
    FIELD_INT32(    gmt_offset,        "0",           -43200,  50400, cbNTPConfigUpdate),
    FIELD_UINT16(   daylight_offset,   "0",                 0, 7200,  cbNTPConfigUpdate),
    FIELD_STRING(   tz_string,         "CET-1CEST,M3.5.0/2,M10.5.0/3",
                                                            0,        cbNTPConfigUpdate),
// ===== Runtime only (not stored in flash) =====
    FIELD_STRING(   info_text,         "",                  0,        nullptr)
 };

 #endif
