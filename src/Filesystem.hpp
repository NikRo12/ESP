#pragma once
#include "esp_littlefs.h"

class FileSystem {
public:
    FileSystem(const char* base_path, const char* partition_label);
    void begin();

private:
    const char* _base_path;
    const char* _partition_label;
};