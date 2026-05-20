#include "Filesystem.hpp"
#include "esp_log.h"

static constexpr const char* TAG = "FileSystem";

FileSystem::FileSystem(const char* base_path, const char* partition_label) : 
    _base_path(base_path),
    _partition_label(partition_label) {}

void FileSystem::begin() {
    esp_vfs_littlefs_conf_t conf = {
        .base_path = _base_path,
        .partition_label = _partition_label,
        .format_if_mount_failed = true,
        .dont_mount = false,
    };

    esp_err_t err = esp_vfs_littlefs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount: %s", esp_err_to_name(err));
        return;
    }

    size_t total, used;
    esp_littlefs_info(_partition_label, &total, &used);
    ESP_LOGI(TAG, "LittleFS mounted: %u/%u bytes used", used, total);
}