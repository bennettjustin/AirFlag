#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdint.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_sleep.h"

#include "sleep_drvr.h"

void enterSleep()
{
    // Re-registering call back in exitSleep(), will this be an issue
    ESP_ERROR_CHECK(esp_bluedroid_disable());

    ESP_ERROR_CHECK(esp_bt_controller_disable());

    esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
    vTaskDelay(200 / portTICK_PERIOD_MS);
    esp_deep_sleep_start();
}

void exitSleep(esp_gap_ble_cb_t esp_gap_cb, esp_ble_scan_params_t *ble_scan_params)
{
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));

    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gap_register_callback(esp_gap_cb));

    ESP_ERROR_CHECK(esp_ble_gap_set_scan_params(ble_scan_params));
}