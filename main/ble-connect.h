#pragma once

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_nimble_hci.h"
#include "nimble/nimble_port.h"
#include "nimble/nimble_port_freertos.h"
#include "host/ble_hs.h"
#include "services/gap/ble_svc_gap.h"
#include "services/gatt/ble_svc_gatt.h"
#include "sdkconfig.h"
#include "ble-connect.h"
#include "wifi-connect.h"


// Constants
#define TAG "BLE-Server"

// Function prototypes
void ble_app_advertise(void);
void ble_app_on_sync(void);
void host_task(void *param);


// GATT service definitions
extern const struct ble_gatt_svc_def gatt_svcs[];