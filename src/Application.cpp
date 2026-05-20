#include "Application.hpp"
#include "endpoints.hpp"
#include <string>
#include "cJSON.h"
#include "esp_timer.h"

Application::Application(uint16_t port, gpio_num_t led1Pin, gpio_num_t led2Pin,
                         gpio_num_t btn1Pin, gpio_num_t btn2Pin,
                         gpio_num_t rgbPin, uint16_t numLeds,
                         adc_channel_t potChannel)
    : _server(port),
      _led1(led1Pin),
      _led2(led2Pin),
      _btn1(btn1Pin),
      _btn2(btn2Pin),
      _rgb(rgbPin, numLeds),
      _adc(),
      _pot(_adc, potChannel) {}

void Application::begin() {
    _led1.init();
    _led2.init();
    _btn1.init();
    _btn2.init();
    _rgb.init();
    _adc.begin();
    _pot.init();

    _server.begin();

    _server.addEndpoint("/", HTTP_GET, endpoints::handleRoot, this);
    _server.addEndpoint("/upload", HTTP_GET, endpoints::handleUploadForm, this);
    _server.addEndpoint("/upload", HTTP_POST, endpoints::handleUpload, this);
    _server.addEndpoint("/switch1", HTTP_POST, endpoints::handleSwitch1, this);
    _server.addEndpoint("/switch2", HTTP_POST, endpoints::handleSwitch2, this);
    _server.addEndpoint("/state", HTTP_GET, endpoints::handleState, this);
    _server.addEndpoint("/rgb", HTTP_POST, endpoints::handleRGB, this);
    _server.addWebSocket("/ws", endpoints::handleWebSocket, this);
}

void Application::markDirty() {
    _dirty = true;
}

void Application::setRgb(uint8_t r, uint8_t g, uint8_t b) {
    int64_t now = esp_timer_get_time() / 1000;
    if (now - _lastRgbUpdate < RGB_THROTTLE_MS) {
        _pendingR = r;
        _pendingG = g;
        _pendingB = b;
        _hasPending = true;
        return;
    }
    _lastRgbUpdate = now;
    _rgb.setColor(r, g, b);
}

void Application::tick() {
    if (_btn1.pollPressed()) {
        _led1.toggle();
        markDirty();
    }
    if (_btn2.pollPressed()) {
        _led2.toggle();
        markDirty();
    }

    if (_pot.poll()) {
        markDirty();
    }

    int64_t now = esp_timer_get_time() / 1000;
    if (_hasPending && (now - _lastRgbUpdate) >= RGB_THROTTLE_MS) {
        _lastRgbUpdate = now;
        _rgb.setColor(_pendingR, _pendingG, _pendingB);
        _hasPending = false;
    }

    if (_dirty) {
        broadcast();
        _dirty = false;
    }
}

void Application::broadcast() {
    std::string state = serializeState();
    _server.broadcast(state.c_str(), state.size());
}

const char* Application::basePath() const { return _basePath; }

Led& Application::led1() { return _led1; }
Led& Application::led2() { return _led2; }
RGB& Application::rgb() { return _rgb; }

std::string Application::serializeState() const {
    cJSON* root = cJSON_CreateObject();
    cJSON_AddBoolToObject(root, "switch1", _led1.isOn());
    cJSON_AddBoolToObject(root, "switch2", _led2.isOn());
    cJSON_AddNumberToObject(root, "lightSensor", _pot.percent());

    cJSON* rgb = cJSON_AddObjectToObject(root, "rgb");
    cJSON_AddNumberToObject(rgb, "r", _rgb.r());
    cJSON_AddNumberToObject(rgb, "g", _rgb.g());
    cJSON_AddNumberToObject(rgb, "b", _rgb.b());

    char* raw = cJSON_PrintUnformatted(root);
    std::string result(raw);
    free(raw);

    cJSON_Delete(root);
    return result;
}