#pragma once
#include <esp_http_server.h>

namespace endpoints {
    esp_err_t handleSwitch1(httpd_req_t* req);
    esp_err_t handleSwitch2(httpd_req_t* req);
    esp_err_t handleState(httpd_req_t* req);
    esp_err_t handleRGB(httpd_req_t* req);
    esp_err_t handleWebSocket(httpd_req_t* req);
    esp_err_t handleUploadForm(httpd_req_t* req);
    esp_err_t handleUpload(httpd_req_t* req);
    esp_err_t handleRoot(httpd_req_t* req);
}