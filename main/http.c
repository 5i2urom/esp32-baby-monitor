#include <stdio.h>                     // Для snprintf
#include <string.h>                    // Для strlen
#include <stdint.h>                    // Для int16_t
#include "esp_http_server.h"           // Для HTTP-сервера
#include "esp_log.h"                   // Для логирования

extern int16_t temperature;  // Глобальные переменные
extern int16_t humidity;
extern int16_t noise_level;
extern int16_t threshold_noise_level;

static esp_err_t http_get_handler(httpd_req_t *req) {
    char response[512];

    snprintf(response, sizeof(response),
             "<!DOCTYPE html>"
             "<html>"
             "<head><title>Baby Monitor</title></head>"
             "<body>"
             "<h1>Baby Monitor Status</h1>"
             "<p>Temperature: %d &#8451;</p>"
             "<p>Humidity: %d %%</p>"
             "<p>Noise Level: %d</p>"
             "</body>"
             "</html>",
             temperature, humidity, noise_level);

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, response, strlen(response));
    return ESP_OK;
}

void start_http_server(void) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t uri_get = {
            .uri       = "/",
            .method    = HTTP_GET,
            .handler   = http_get_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &uri_get);
    } else {
        ESP_LOGE("HTTP", "Failed to start HTTP server");
    }
}

