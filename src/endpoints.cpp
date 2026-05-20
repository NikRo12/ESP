#include "endpoints.hpp"
#include "Application.hpp"
#include "esp_log.h"
#include <string>
#include <cJSON.h>
#include <sys/stat.h>

static constexpr const char* TAG = "Endpoints";
static constexpr const char* UPLOAD_FORM = R"html(
<!DOCTYPE html>
<html>
<body>
<input type="file" id="f">
<button onclick="upload()">Upload</button>
<script>
async function upload() {
    const file = document.getElementById('f').files[0];
    await fetch('/upload', { method: 'POST', body: file });
    alert('Uploaded');
}
</script>
</body>
</html>
)html";

static constexpr const char* NO_INDEX_PAGE = R"html(
<!DOCTYPE html>
<html>
<body>
<h1>UI is not loaded</h1>
<p><a href="/upload">Go to upload page</a></p>
</body>
</html>
)html";

static esp_err_t toggleLed(httpd_req_t* req, Application& app, Led& led) {
    Server::setCorsHeaders(req);
    httpd_resp_set_type(req, "application/json");
    led.toggle();
    app.markDirty();
    httpd_resp_sendstr(req, led.isOn() ? "true" : "false");
    return ESP_OK;
}

static esp_err_t lightRGB(httpd_req_t* req, Application& app, RGB& rgb) {
    Server::setCorsHeaders(req);
    httpd_resp_set_type(req, "application/json");

    char buffer[128];
    int received = httpd_req_recv(req, buffer, sizeof(buffer) - 1);
    if (received <= 0) {
        ESP_LOGE(TAG, "Failed to read body: %d", received);
        return ESP_FAIL;
    }
    buffer[received] = '\0';

    cJSON* root = cJSON_Parse(buffer);
    if (root == nullptr) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    cJSON* rField = cJSON_GetObjectItem(root, "r");
    cJSON* gField = cJSON_GetObjectItem(root, "g");
    cJSON* bField = cJSON_GetObjectItem(root, "b");

    if (!cJSON_IsNumber(rField) || !cJSON_IsNumber(gField) || !cJSON_IsNumber(bField)) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing or non-numeric fields");
        return ESP_FAIL;
    }

    int r = rField->valueint;
    int g = gField->valueint;
    int b = bField->valueint;

    if (r < 0 || r > 255 || g < 0 || g > 255 || b < 0 || b > 255) {
        cJSON_Delete(root);
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Values must be 0-255");
        return ESP_FAIL;
    }

    rgb.setColor(static_cast<uint8_t>(r),
                 static_cast<uint8_t>(g),
                 static_cast<uint8_t>(b));
    app.markDirty();
    cJSON_Delete(root);

    char response[64];
    snprintf(response, sizeof(response), "{\"r\":%d,\"g\":%d,\"b\":%d}", r, g, b);
    httpd_resp_sendstr(req, response);
    return ESP_OK;
}

static esp_err_t getState(httpd_req_t* req, Application& app) {
    Server::setCorsHeaders(req);
    httpd_resp_set_type(req, "application/json");

    std::string state = app.serializeState();
    httpd_resp_send(req, state.c_str(), state.size());
    return ESP_OK;
}

static Application* appFrom(httpd_req_t* req, const char* handler) {
    ESP_LOGI(TAG, "%s called", handler);
    Application* app = static_cast<Application*>(req->user_ctx);
    if (app == nullptr) ESP_LOGE(TAG, "user_ctx is null");
    return app;
}

esp_err_t endpoints::handleSwitch1(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    return app ? toggleLed(req, *app, app->led1()) : ESP_FAIL;
}

esp_err_t endpoints::handleSwitch2(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    return app ? toggleLed(req, *app, app->led2()) : ESP_FAIL;
}

esp_err_t endpoints::handleState(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    return app ? getState(req, *app) : ESP_FAIL;
}

esp_err_t endpoints::handleRGB(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    return app ? lightRGB(req, *app, app->rgb()) : ESP_FAIL;
}

esp_err_t endpoints::handleWebSocket(httpd_req_t* req) {
    ESP_LOGI(TAG, "%s called", __func__);
    if (req->method == HTTP_GET) {
        ESP_LOGI(TAG, "WS client connected");
    }
    return ESP_OK;
}

esp_err_t endpoints::handleUploadForm(httpd_req_t* req) {
    ESP_LOGI(TAG, "%s called", __func__);
    Server::setCorsHeaders(req);
    httpd_resp_set_type(req, "text/html");
    httpd_resp_sendstr(req, UPLOAD_FORM);
    return ESP_OK;
}

esp_err_t endpoints::handleUpload(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    if (!app) return ESP_FAIL;

    ESP_LOGI(TAG, "content_len=%zu", req->content_len);
    Server::setCorsHeaders(req);

    //ХАРДКОД ПУТИ, ИНАЧЕ КОР ДАМП ПОЧЕМУ ТО
    std::string path = std::string("/littlefs/index.html");

    FILE* file = fopen(path.c_str(), "w");
    if (file == nullptr) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot open file");
        return ESP_FAIL;
    }

    size_t remaining = req->content_len;
    char buf[1024];

    while (remaining > 0) {
        size_t toRead = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        int received = httpd_req_recv(req, buf, toRead);

        if (received <= 0) {
            if (received == HTTPD_SOCK_ERR_TIMEOUT) {
                continue;
            }
            ESP_LOGE(TAG, "Failed to receive body: %d", received);
            fclose(file);
            return ESP_FAIL;
        }

        size_t written = fwrite(buf, 1, received, file);
        if (written != static_cast<size_t>(received)) {
            ESP_LOGE(TAG, "Failed to write: wrote %zu of %d", written, received);
            fclose(file);
            return ESP_FAIL;
        }

        remaining -= received;
    }

    fclose(file);
    ESP_LOGI(TAG, "Uploaded %u bytes to %s", req->content_len, path.c_str());
    httpd_resp_set_type(req, "application/json");
    httpd_resp_sendstr(req, "{\"ok\":true}");
    return ESP_OK;
}

esp_err_t endpoints::handleRoot(httpd_req_t* req) {
    Application* app = appFrom(req, __func__);
    if (!app) return ESP_FAIL;

    Server::setCorsHeaders(req);
    httpd_resp_set_type(req, "text/html");

    //ХАРДКОД ПУТИ, ИНАЧЕ КОР ДАМП ПОЧЕМУ ТО
    std::string path = std::string("/littlefs/index.html"); 

    struct stat st;
    if (stat(path.c_str(), &st) != 0) {
        httpd_resp_sendstr(req, NO_INDEX_PAGE);
        return ESP_OK;
    }

    FILE* file = fopen(path.c_str(), "r");
    if (file == nullptr) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Cannot open file");
        return ESP_FAIL;
    }

    char buf[1024];
    size_t bytes;
    while ((bytes = fread(buf, 1, sizeof(buf), file)) > 0) {
        if (httpd_resp_send_chunk(req, buf, bytes) != ESP_OK) {
            fclose(file);
            return ESP_FAIL;
        }
    }

    fclose(file);
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_OK;
}
