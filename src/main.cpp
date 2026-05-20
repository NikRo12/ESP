#include "platform.hpp"
#include "Filesystem.hpp"
#include "Wifi.hpp"
#include "Application.hpp"
#include "RGB.hpp"

// extern "C" void app_main() {
//     platformInit();

//     static constexpr const char* FS_BASE_PATH = "/littlefs";

//     static FileSystem fs(FS_BASE_PATH, "storage");
//     fs.begin();

//     static WifiAP wifi("qwe", "12345678");
//     wifi.begin();

//     static Application app(80, GPIO_NUM_3, GPIO_NUM_4, GPIO_NUM_9, GPIO_NUM_6, GPIO_NUM_5, 64, ADC_CHANNEL_0);
//     app.begin();

//     while (true) {
//         vTaskDelay(pdMS_TO_TICKS(100));
//         app.tick();
//     }
// }

extern "C" void app_main() {
    platformInit();

    RGB rgb(GPIO_NUM_5, 64);
    rgb.init();

    while (true) {
        vTaskDelay(pdMS_TO_TICKS(100));
        rgb.setColor(199, 151, 54);
        vTaskDelay(pdMS_TO_TICKS(1000));
        rgb.setColor(0, 0, 0);
    }
}
