#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "dht.h"
#include "driver/adc.h"
#include "i2c-lcd.h"
#include "ble-connect.h"
#include <esp_wifi.h>
#include "http.h"

#define I2C_MASTER_SCL_IO           GPIO_NUM_22     
#define I2C_MASTER_SDA_IO           GPIO_NUM_21      
#define I2C_MASTER_NUM              0                    
#define I2C_MASTER_FREQ_HZ          400000                    
#define I2C_MASTER_TX_BUF_DISABLE   0                         
#define I2C_MASTER_RX_BUF_DISABLE   0                          
#define I2C_MASTER_TIMEOUT_MS       1000
#define SENSOR_TYPE                 DHT_TYPE_DHT11
#define DHT_GPIO                    GPIO_NUM_4
#define KY037_AO_PIN                ADC1_CHANNEL_6 // GPIO34
#define ADC_MAX_VALUE               4095 

#define WIFI_TAG "WIFI"

int16_t temperature = 0;
int16_t humidity = 0;
int16_t noise_level = 0;
int16_t threshold_noise_level = 0; 

extern char ip_str[16];
static char empty_str[13];

static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_MASTER_NUM;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
}

// void put_led_string(char line, char row, char* buffer) {
//     lcd_set_cursor(line, row);
//     lcd_send_string(empty_str);
//     lcd_set_cursor(line, row);
//     lcd_send_string(buffer);
// }
SemaphoreHandle_t lcd_mutex;

void put_led_string(char line, char row, char* buffer) {
    if (xSemaphoreTake(lcd_mutex, portMAX_DELAY)) { // Берем мьютекс
        lcd_set_cursor(line, row);
        lcd_send_string(empty_str);
        lcd_set_cursor(line, row);
        lcd_send_string(buffer);
        xSemaphoreGive(lcd_mutex); // Освобождаем мьютекс
    } else {
        ESP_LOGE(TAG, "Failed to take LCD mutex");
    }
}

void dht_task(void *pvParameters) {
    int16_t temperature = 0, humidity = 0;
    char buffer[13];

    gpio_set_direction(DHT_GPIO, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(DHT_GPIO, 1);

    do
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        dht_read_data(SENSOR_TYPE, DHT_GPIO, &humidity, &temperature);

        temperature /= 10;
        humidity /= 10;

        sprintf(buffer, "%d C", temperature);
        put_led_string(0, 8, buffer);

        sprintf(buffer, "%d %%", humidity);
        put_led_string(1, 8, buffer);
        
    } while (1);
    
}

void ky037_task(void *pvParameters) {
    int noise_level = 0;
    int16_t inverted_noise_level = 0, period_max_level = 0;

    char buffer[20];

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(KY037_AO_PIN, ADC_ATTEN_DB_12);

    while (1) { 
        period_max_level = 0;
        for (int i = 0; i < 10; ++i) {
            noise_level = adc1_get_raw(KY037_AO_PIN);
            inverted_noise_level = ADC_MAX_VALUE - noise_level; 
            if (inverted_noise_level > period_max_level)
                period_max_level = inverted_noise_level;
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        
        if (period_max_level < threshold_noise_level) {
            sprintf(buffer, "%d/%d", period_max_level, threshold_noise_level);
            put_led_string(2, 8, buffer);
        } else {
            for (int j = 0; j < 10; ++j) {
                sprintf(buffer, "%d/%d !", period_max_level, threshold_noise_level);
                put_led_string(2, 8, buffer);
                vTaskDelay(pdMS_TO_TICKS(500));

                sprintf(buffer, "%d/%d", period_max_level, threshold_noise_level);
                put_led_string(2, 8, buffer);
                vTaskDelay(pdMS_TO_TICKS(500));
            }
        }        
    }
}

void read_noise_level_from_nvs() {
    nvs_handle_t nvs_handle;
    esp_err_t err;

    // Открываем NVS для чтения
    err = nvs_open("storage", NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to open NVS: %s", esp_err_to_name(err));
    }

    // Считываем значение уровня шума
    err = nvs_get_i16(nvs_handle, "noise_level", &threshold_noise_level);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGW("NVS", "Noise level not found in NVS, using default.");
    } else if (err != ESP_OK) {
        ESP_LOGE("NVS", "Failed to read noise level: %s", esp_err_to_name(err));
    }

    // Закрываем NVS
    nvs_close(nvs_handle);

    ESP_LOGI("NVS", "Read noise level from NVS: %d", threshold_noise_level);
}

void app_main(void)
{    
    char buffer[21];
    memset(empty_str, ' ', 12);
    empty_str[12] = '\0';

    lcd_mutex = xSemaphoreCreateMutex();
    if (lcd_mutex == NULL) {
        ESP_LOGE(TAG, "Failed to create mutex for LCD");
    }

    i2c_master_init();
    lcd_init();
    lcd_clear();

    
    nvs_flash_init();                          // 1 - Initialize NVS flash using
    read_noise_level_from_nvs();

    sprintf(buffer, "Temp  :");
    put_led_string(0, 0, buffer);

    sprintf(buffer, "Humi  :");
    put_led_string(1, 0, buffer);

    sprintf(buffer, "Noise :");
    put_led_string(2, 0, buffer);

    xTaskCreate(dht_task, "DHT Task", 4096, NULL, 3, NULL);
    xTaskCreate(ky037_task, "KY037 Task", 4096, NULL, 3, NULL);   

    // esp_nimble_hci_and_controller_init();      // 2 - Initialize ESP controller
    nimble_port_init();                        // 3 - Initialize the host stack
    ble_svc_gap_device_name_set("BLE-Server"); // 4 - Initialize NimBLE configuration - server name
    ble_svc_gap_init();                        // 4 - Initialize NimBLE configuration - gap service
    ble_svc_gatt_init();                       // 4 - Initialize NimBLE configuration - gatt service
    ble_gatts_count_cfg(gatt_svcs);            // 4 - Initialize NimBLE configuration - config gatt services
    ble_gatts_add_svcs(gatt_svcs);             // 4 - Initialize NimBLE configuration - queues gatt services.
    ble_hs_cfg.sync_cb = ble_app_on_sync;      // 5 - Initialize application
    nimble_port_freertos_init(host_task);      // 6 - Run the thread 

    init_wifi();
    char ssid[32];
    char password[64];    
    if (get_wifi_credentials(ssid, sizeof(ssid), password, sizeof(password)) == ESP_OK) {
        connect_to_wifi(ssid, password);
    } else {
        ESP_LOGW(WIFI_TAG, "No Wi-Fi credentials found. Waiting for configuration...");
    }

    while (strlen(ip_str) == 0) {
        ESP_LOGI("MAIN", "Waiting for IP address...");
        vTaskDelay(pdMS_TO_TICKS(1000));  // Задержка 1 секунда
    }

    ESP_LOGI(WIFI_TAG, "Wi-Fi connected. Starting HTTP server...");
    start_http_server();
}
