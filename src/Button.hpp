#pragma once
#include "driver/gpio.h"
#include "esp_timer.h"

class Button {
public:
    explicit Button(gpio_num_t pin) : _pin(pin) {}

    void init();
    bool pollPressed();

private:
    static constexpr int64_t DEBOUNCE_MS = 50;

    gpio_num_t _pin;
    bool _lastState = false;
    int64_t _lastChange = 0;
};
