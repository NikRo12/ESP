#pragma once
#include <cstdint>
#include <esp_http_server.h>

class Server {
public:
    using HttpHandler = esp_err_t(*)(httpd_req_t*);

    explicit Server(uint16_t port);
    void begin();
    void addEndpoint(const char* path, httpd_method_t method, HttpHandler handler, void* ctx);
    void addWebSocket(const char* path, HttpHandler handler, void* ctx);
    static void setCorsHeaders(httpd_req_t* req);

    void broadcast(const char* data, size_t len);

private:
    httpd_handle_t _server = nullptr;
    uint16_t _port;
};