#pragma once
#include "esp_adc/adc_oneshot.h"

class AdcManager {
public:
    void begin();
    void configureChannel(adc_channel_t channel, adc_atten_t atten);
    int read(adc_channel_t channel);

private:
    adc_oneshot_unit_handle_t _handle = nullptr;
};