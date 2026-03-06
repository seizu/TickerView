#ifndef NETWORK_H
#define NETWORK_H

#define HTTP_PAYLOAD_BUFFER    512

struct HttpResult {
    int code;
    size_t bytes_read;
    char payload[HTTP_PAYLOAD_BUFFER];
};

bool httpGet(const String &url, HttpResult &result, const uint16_t time_out = 3000);
void initWifi();
void handleWiFi();
void updateNTP();

extern HttpResult result;

#endif // NETWORK_H
