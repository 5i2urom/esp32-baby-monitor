#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"
#include <string.h>
#include "wifi-connect.h"

#define WIFI_TAG "WIFI"

char ip_str[16] = {0};

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(WIFI_TAG, "Wi-Fi started, attempting to connect...");
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        //ESP_LOGE(WIFI_TAG, "Wi-Fi disconnected, retrying...");
        wifi_event_sta_disconnected_t *disconnected = (wifi_event_sta_disconnected_t *)event_data;
        ESP_LOGE(WIFI_TAG, "Wi-Fi disconnected, reason=%d", disconnected->reason);
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Connected! IP Address: " IPSTR, IP2STR(&event->ip_info.ip));
        snprintf(ip_str, sizeof(ip_str), IPSTR, IP2STR(&event->ip_info.ip));
    }
}

void init_wifi() {
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &instance_any_id);
    esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &instance_got_ip);

    ESP_LOGI(WIFI_TAG, "Wi-Fi initialized");
}

esp_err_t get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len) {
    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to open NVS: %s", esp_err_to_name(err));
        return err;
    }

    err = nvs_get_str(nvs_handle, "wifi_ssid", ssid, &ssid_len);
    if (err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to read SSID from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    err = nvs_get_str(nvs_handle, "wifi_pass", password, &password_len);
    if (err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to read Password from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return err;
    }

    nvs_close(nvs_handle);

    ESP_LOGI(WIFI_TAG, "Retrieved Wi-Fi credentials: SSID=%s, Password=%s", ssid, password);

    return ESP_OK;
}


esp_err_t connect_to_wifi(const char *ssid, const char *password) {
    // Настройки подключения
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = "",
        },
    };
    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    ESP_LOGI(WIFI_TAG, "Connecting to Wi-Fi network: %s", ssid);

    // Устанавливаем конфигурацию Wi-Fi
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);

    // Запускаем Wi-Fi
    esp_wifi_start();

    // Подключаемся к сети
    return esp_wifi_connect();
}

void update_wifi_credentials(const char *new_ssid, const char *new_password) {
    char current_ssid[32] = {0};
    char current_password[64] = {0};
    size_t ssid_len = sizeof(current_ssid);
    size_t password_len = sizeof(current_password);

    nvs_handle_t nvs_handle;
    esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err == ESP_OK) {
        nvs_get_str(nvs_handle, "wifi_ssid", current_ssid, &ssid_len);
        nvs_get_str(nvs_handle, "wifi_pass", current_password, &password_len);
        nvs_close(nvs_handle);
    } else if (err != ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGE(WIFI_TAG, "Failed to open NVS for reading: %s", esp_err_to_name(err));
        return;
    }

    // Проверяем, изменились ли данные
    if (strcmp(new_ssid, current_ssid) == 0 && strcmp(new_password, current_password) == 0) {
        ESP_LOGI(WIFI_TAG, "Credentials are the same. No need to reconnect.");
        return;
    }

    // Сохраняем новые данные в NVS
    err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(WIFI_TAG, "Failed to open NVS for writing: %s", esp_err_to_name(err));
        return;
    }

    nvs_set_str(nvs_handle, "wifi_ssid", new_ssid);
    nvs_set_str(nvs_handle, "wifi_pass", new_password);
    nvs_commit(nvs_handle);
    nvs_close(nvs_handle);

    ESP_LOGI(WIFI_TAG, "Wi-Fi credentials updated: SSID=%s", new_ssid);

    esp_wifi_disconnect();
    connect_to_wifi(new_ssid, new_password);

}