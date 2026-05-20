#include "Wifi.hpp"
#include <cstdint>
#include <cstring>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"

WifiAP::WifiAP(const char* ssid, const char* password) : _ssid(ssid), _password(password) {}

void WifiAP::begin() {
    esp_netif_create_default_wifi_ap(); //create virtual net-card
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); //default big config for net
    esp_wifi_init(&cfg); //start wifi driver, but not activate

    esp_wifi_set_mode(WIFI_MODE_AP);

    wifi_config_t wifi_config = {};
    strncpy((char*)wifi_config.ap.ssid, _ssid, sizeof(wifi_config.ap.ssid) - 1);
    strncpy((char*)wifi_config.ap.password, _password, sizeof(wifi_config.ap.password) - 1);
    wifi_config.ap.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.ap.max_connection = 4;

    esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
    esp_wifi_start();
}