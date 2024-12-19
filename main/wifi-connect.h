#pragma once

#include <stdint.h>
#include <stddef.h>
#include "esp_err.h"

void init_wifi(void);
esp_err_t connect_to_wifi(const char *ssid, const char *password); 
esp_err_t get_wifi_credentials(char *ssid, size_t ssid_len, char *password, size_t password_len); 
void update_wifi_credentials(const char *new_ssid, const char *new_password);