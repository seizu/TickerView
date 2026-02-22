#ifndef RTCLOCK_H
#define RTCLOCK_H

#include <Arduino.h>  // Required for PlatformIO
#include <time.h>
#include "serlog.h"

class RealTimeClock {
  public:
    bool syncNTP();
    void setNTP(const char* ntp_server, const long gmt_offset, int daylight_offset); 
    void setNTPWithTimezone(const char* ntp_server, const char* tz_string);
    void setTimestamp(uint32_t unix_timestamp);
    void setTimestamp(const char *date_time, bool utc = true);
    ulong getLocalTimestamp();
    ulong getTimestamp();
    void updateTimestamp();
    String getFormattedTimestamp();
    String getFormattedTimestamp(const char * format);

  private:
    uint32_t localtime_offset = 0;
    uint32_t stored_timestamp = 0;
    uint32_t last_update = 0;
    bool use_posix_tz = false;
};

#endif // RTCLOCK_H
