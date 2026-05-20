#include "RGB.hpp"
#include "esp_log.h"

static constexpr const char* TAG = "RGB";

RGB::RGB(gpio_num_t pin, uint16_t numLeds) : 
    _pin(pin),
    _numLeds(numLeds) {}


void RGB::init() {
    led_strip_config_t stripConfig = {
        .strip_gpio_num = _pin,
        .max_leds = _numLeds,
        .led_model = LED_MODEL_WS2812,
    };

    led_strip_rmt_config_t rmtConfig = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000
    };

    esp_err_t err = led_strip_new_rmt_device(&stripConfig, &rmtConfig, &_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create rgb: %s", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Created rgb");
        led_strip_clear(_handle);
    }
}

void RGB::setColor(uint8_t r, uint8_t g, uint8_t b) {
    if (_handle == nullptr) {
        ESP_LOGE(TAG, "setColor called before init");
        return;
    }

    _r = r;
    _g = g;
    _b = b;

    for (uint16_t i = 0; i < _numLeds; ++i) {
        led_strip_set_pixel(_handle, i, r, g, b);
    }

    esp_err_t err = led_strip_refresh(_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Refresh failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "Color set to (%u, %u, %u)", r, g, b);
}

uint8_t RGB::r() const { return _r; }
uint8_t RGB::g() const { return _g; }
uint8_t RGB::b() const { return _b; }