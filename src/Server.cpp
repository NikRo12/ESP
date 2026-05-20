#include "Server.hpp"
#include "esp_log.h"

static constexpr const char* TAG = "Server";

Server::Server(uint16_t port) : _port(port) {}

void Server::begin() {
    httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
    cfg.server_port = _port;
    cfg.max_uri_handlers = 16;

    esp_err_t err = httpd_start(&_server, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start server on port %u: %s", _port, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Server started on port: %u", _port);
    }
}

void Server::addEndpoint(const char* path, httpd_method_t method, HttpHandler handler, void* ctx) {
    httpd_uri_t uri = {
        .uri = path,
        .method = method,
        .handler = handler,
        .user_ctx = ctx
    };

    esp_err_t err = httpd_register_uri_handler(_server, &uri);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register endpoint %s: %s", path, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Successfully registered handler: %s", path);
    }
}

void Server::addWebSocket(const char* path, HttpHandler handler, void* ctx) {
    httpd_uri_t uri = {
        .uri = path,
        .method = HTTP_GET,
        .handler = handler,
        .user_ctx = ctx,
        .is_websocket = true,
    };

    esp_err_t err = httpd_register_uri_handler(_server, &uri);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to register endpoint %s: %s", path, esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "Successfully registered handler: %s", path);
    }
}

void Server::setCorsHeaders(httpd_req_t* req) {
    httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
    httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");
}

void Server::broadcast(const char* data, size_t len) {
    httpd_ws_frame_t ws_pkt = {};
    ws_pkt.type = HTTPD_WS_TYPE_TEXT;
    ws_pkt.payload = (uint8_t*)data;
    ws_pkt.len = len;

    constexpr size_t max_clients = CONFIG_LWIP_MAX_LISTENING_TCP;
    int client_fds[max_clients];
    size_t fds_count = max_clients;

    esp_err_t err = httpd_get_client_list(_server, &fds_count, client_fds);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to get client list: %s", esp_err_to_name(err));
        return;
    }
    uint8_t count = 0;

    for(size_t i = 0; i < fds_count; i++) {
        int fd = client_fds[i];
        httpd_ws_client_info_t info = httpd_ws_get_fd_info(_server, fd);
        if (info != HTTPD_WS_CLIENT_WEBSOCKET) {
            continue;
        }
        
        esp_err_t r = httpd_ws_send_frame_async(_server, fd, &ws_pkt);
        if (r == ESP_OK) {
            count++;
        } else {
            ESP_LOGW(TAG, "Failed to send to fd=%d: %s", fd, esp_err_to_name(r));
        }
    }

    ESP_LOGD(TAG, "Broadcast: sent to %u WS clients", count);
}