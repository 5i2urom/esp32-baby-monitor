#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

// Макрос для лога
#define WIFI_TAG "WIFI"

// Функции для работы с Wi-Fi
void init_wifi(void); // Инициализация Wi-Fi
esp_err_t connect_to_wifi(const char *ssid, const char *password); // Подключение к Wi-Fi
esp_err_t get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len); // Получение сохранённых данных Wi-Fi
void update_wifi_credentials(const char *new_ssid, const char *new_password); // Обновление данных Wi-Fi