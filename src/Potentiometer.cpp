#include "Potentiometer.hpp"
#include "esp_log.h"
#include <cstdlib>

static constexpr const char* TAG = "Potentiometer";
static constexpr int DEADBAND = 30;

Potentiometer::Potentiometer(AdcManager& adc, adc_channel_t channel)
    : _adc(adc), _channel(channel) {}

void Potentiometer::init() {
    _adc.configureChannel(_channel, ADC_ATTEN_DB_12);
    ESP_LOGI(TAG, "Potentiometer on channel %d initialized", _channel);
}

bool Potentiometer::poll() {
    int newValue = _adc.read(_channel);
    if (newValue < 0) return false;

    if (std::abs(newValue - _value) > DEADBAND) {
        _value = newValue;
        return true;
    }
    return false;
}

int Potentiometer::raw() const { return _value; }
int Potentiometer::percent() const { return _value * 100 / 4095; }