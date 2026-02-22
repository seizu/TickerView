#include "globals.h"
#include "storage.h"


// ====================================================================================================
// WebPrefs lifecycle callbacks =======================================================================
// ====================================================================================================
void cbNTPConfigUpdate() {
    if(*dc.tz_string == '\0') {
        rtc.setNTP(dc.ntp_server, dc.gmt_offset, dc.daylight_offset);
    } else {
        rtc.setNTPWithTimezone(dc.ntp_server, dc.tz_string);
    }
}

// void cbSetClock() {
//     LOG_SDEBUG("Set Clock: %s", dc.date_time);
//     rtc.setTimestamp(dc.date_time);
// }


void cbBeforeResponse() {
    LOG_SDEBUG("Before response");
    snprintf(dc.info_text, sizeof(dc.info_text), "TickerView V%s (WebPrefs V%s)", TICKERVIEW_VERSION, WEBPREFS_VERSION);
}

void cbSave(int params_count) {
    // to reduce write cycles, only multiple parameters are written, updates of a single parameter are ignored.
    if (params_count > 1) {
        if (writeConfig()) {
            LOG_SINFO("Config written! ChkSum: 0x%X", dc.check_sum);
        } else {
            LOG_SERROR("Save config failed!");
        }
    }
}

void cbDone() {
    LOG_SINFO("reboot");
    delay(10);
    ESP.restart();
}



/**
 * Write EEPROM flash storage
**/
bool writeConfig() {
    uint8_t *buffer = (uint8_t *)&dc;
    size_t size = offsetof(DeviceConfig, __STORAGE_END__);
    const size_t skip = sizeof(dc.header) + sizeof(dc.check_sum);
    dc.check_sum = computeChecksum((buffer + skip), size - skip);
    snprintf(dc.header, 4, "%s", "PET");
    EEPROM.begin(size);
    for (size_t i = 0; i < size; i++) {
        EEPROM.write(i, buffer[i]);
    }
    bool result = EEPROM.commit();
    EEPROM.end();
    return result;
}

/**
 * Read EEPROM flash storage
**/
bool readConfig() {
    uint8_t *buffer = (uint8_t *)&dc;
    size_t size = offsetof(DeviceConfig, __STORAGE_END__);
    EEPROM.begin(size);
    for (size_t i = 0; i < size; i++) {
        buffer[i] = EEPROM.read(i);
    }
    EEPROM.end();
    const size_t skip = sizeof(dc.header) + sizeof(dc.check_sum);
    uint16_t chk_sum = computeChecksum((buffer + skip), size - skip);
    if (chk_sum == dc.check_sum && strncmp(dc.header, "PET\0", 4) == 0) {
        return true;
    }
    return false;
}

uint16_t computeChecksum(const uint8_t *data, size_t size) {
    uint16_t checksum = size;
    while (size) {
        checksum += *data++ * size--;
    }
    return checksum;
}

/**
 * init WebPrefs server on port 80 & load config
**/
void initWebPrefs() {
    LOG_SINFO("--- Init WebPrefs and load config ---");
    wp.begin(80, (void *)&dc, input_fields, cbDone, cbSave, cbBeforeResponse);

    bool valid_config = false;
#if defined READCONFIG
    // read & verify EEPROM config
    if ((valid_config = readConfig())) {
        LOG_SINFO("Config OK! ChkSum: 0x%X", dc.check_sum);
    } else {
        LOG_SERROR("Config NOK! ChkSum: 0x%X", dc.check_sum);
    }
#endif

    // verify config
    if (valid_config == false) {
        // use defaults config
        LOG_SINFO("Set default config");
        wp.setDefaults();
    } else {
        //wp.debugPrintConfig();
    }
    wp.debugPrintConfig();
}