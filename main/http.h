#ifndef BABY_MONITOR_HTTP_H
#define BABY_MONITOR_HTTP_H

#include <stdint.h>                    // Для int16_t
#include "esp_err.h"                   // Для определения типа esp_err_t

#ifdef __cplusplus
extern "C" {
#endif

// Глобальные переменные, которые должны быть определены в другом модуле
extern int16_t temperature;
extern int16_t humidity;
extern int16_t noise_level;
extern int16_t threshold_noise_level;

// Функция для запуска HTTP-сервера
void start_http_server(void);

#ifdef __cplusplus
}
#endif

#endif // BABY_MONITOR_HTTP_H
