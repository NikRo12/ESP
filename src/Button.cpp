#include "Button.hpp"

void Button::init() {
    gpio_reset_pin(_pin);
    gpio_set_direction(_pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(_pin, GPIO_PULLUP_ONLY);
}

bool Button::pollPressed() {
    bool current = (gpio_get_level(_pin) == 0);
    int64_t now = esp_timer_get_time() / 1000;

    if (current != _lastState && (now - _lastChange) > DEBOUNCE_MS) {
        _lastChange = now;
        _lastState = current;
        return current; // true только на переходе HIGH→LOW (нажатие)
    }

    return false;
}
