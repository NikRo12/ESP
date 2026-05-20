#pragma once
#include <cstdint>
#include <string>
#include "driver/gpio.h"
#include "Led.hpp"
#include "Button.hpp"
#include "Server.hpp"
#include "RGB.hpp"
#include "AdcManager.hpp"
#include "Potentiometer.hpp"

class Application {
public:
    Application(uint16_t port, gpio_num_t led1Pin, gpio_num_t led2Pin,
                gpio_num_t btn1Pin, gpio_num_t btn2Pin,
                gpio_num_t rgbPin, uint16_t numLeds,
                adc_channel_t potChannel);

    void begin();
    void markDirty();
    void tick();
    void broadcast();
    void setRgb(uint8_t r, uint8_t g, uint8_t b);

    Led& led1();
    Led& led2();
    RGB& rgb();

    std::string serializeState() const;
    const char* basePath() const;

private:
    static constexpr int64_t RGB_THROTTLE_MS = 50;

    const char* _basePath;
    Server _server;
    Led _led1;
    Led _led2;
    Button _btn1;
    Button _btn2;
    RGB _rgb;
    AdcManager _adc;
    Potentiometer _pot;
    bool _dirty = false;

    int64_t _lastRgbUpdate = 0;
    uint8_t _pendingR = 0;
    uint8_t _pendingG = 0;
    uint8_t _pendingB = 0;
    bool _hasPending = false;
};