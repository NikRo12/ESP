#include "AdcManager.hpp"
#include "esp_log.h"

static constexpr const char* TAG = "AdcManager";

void AdcManager::begin() {
    adc_oneshot_unit_init_cfg_t init_cfg = {};
    init_cfg.unit_id = ADC_UNIT_1;
    esp_err_t err = adc_oneshot_new_unit(&init_cfg, &_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to init ADC: %s", esp_err_to_name(err));
        return;
    }
    ESP_LOGI(TAG, "ADC initialized");
}

void AdcManager::configureChannel(adc_channel_t channel, adc_atten_t atten) {
    adc_oneshot_chan_cfg_t cfg = {};
    cfg.atten = atten;
    cfg.bitwidth = ADC_BITWIDTH_DEFAULT;
    esp_err_t err = adc_oneshot_config_channel(_handle, channel, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure channel %d: %s", channel, esp_err_to_name(err));
    }
}

int AdcManager::read(adc_channel_t channel) {
    int raw = 0;
    esp_err_t err = adc_oneshot_read(_handle, channel, &raw);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to read channel %d: %s", channel, esp_err_to_name(err));
        return -1;
    }
    return raw;
}