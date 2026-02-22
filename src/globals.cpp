#include "globals.h"

// --- global instances ---
DeviceConfig dc;
RealTimeClock rtc;
SimpleWifi sw;
WebPrefs wp;

ulong ntp_update_event = 0;
bool ntp_updated = false;