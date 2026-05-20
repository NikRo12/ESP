#pragma once

class WifiAP {
public:
    WifiAP(const char* ssid, const char* password);
    void begin();
private:
    const char* _ssid;
    const char* _password;
};