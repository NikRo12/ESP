#pragma once
#include "esp_adc/adc_oneshot.h"
#include "AdcManager.hpp"

class Potentiometer {
public:
    Potentiometer(AdcManager& adc, adc_channel_t channel);
    void init();
    bool poll();
    int raw() const;
    int percent() const;

private:
    AdcManager& _adc;
    adc_channel_t _channel;
    int _value = 0;
};