#pragma once
#include "driver/gpio.h"
#include "esp_log.h"

class Led {
public:
    explicit Led(gpio_num_t pin) : _pin(pin), _state(false) {}

    void init() {
        gpio_reset_pin(_pin);
        gpio_set_direction(_pin, GPIO_MODE_OUTPUT);
        ESP_LOGI(TAG, "Led GPIO%u inited", static_cast<unsigned>(_pin));
    }

    void on() {
        gpio_set_level(_pin, 1);
        _state = true;
        ESP_LOGI(TAG, "Led GPIO%u on", static_cast<unsigned>(_pin));
    }

    void off() {
        gpio_set_level(_pin, 0);
        _state = false;
        ESP_LOGI(TAG, "Led GPIO%u off", static_cast<unsigned>(_pin));
    }

    void toggle() {
        _state = !_state;
        gpio_set_level(_pin, _state);
        ESP_LOGI(TAG, "Led GPIO%u toggled", static_cast<unsigned>(_pin));
    }

    bool isOn() const { return _state; }

private:
    static constexpr const char* TAG = "Led";

    gpio_num_t _pin;
    bool _state;
};