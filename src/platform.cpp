#include "platform.hpp"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

void platformInit() {
    nvs_flash_init(); //the flesh key-value memory for a driver usage
    esp_netif_init(); //the net stack
    esp_event_loop_create_default(); //the event loop
}