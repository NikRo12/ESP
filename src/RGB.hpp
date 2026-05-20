#pragma once
#include "driver/gpio.h"
#include "led_strip.h"

class RGB {
public:
    RGB(gpio_num_t pin, uint16_t numLeds);
    void init();
    void setColor(uint8_t r, uint8_t g, uint8_t b);
    
    uint8_t r() const;
    uint8_t g() const;
    uint8_t b() const;

private:
    gpio_num_t _pin;
    uint16_t _numLeds;
    uint8_t _r = 0, _g = 0, _b = 0;
    led_strip_handle_t _handle = nullptr;
};