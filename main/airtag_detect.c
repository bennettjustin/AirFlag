/************************************************
 * Title:      AirTag Detector
 * Decription: To use the BLE functionality to monitor nearby
 *             BLE advertisements in order to detect Disconencted 
 *             AirTags that may be used to covertly track items/people 
 * Date:       12/30/21
 * 
 *************************************************/

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs.h"
#include "nvs_flash.h"
#include <stdint.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"

#include "alert_drvr.h"
#include "sleep_drvr.h"

#define PRINT_OUT_OTHER_DEVICES 0

#define GAP_SCAN_TAG "GAP SCAN"
#define STARTUP_TAG  "STARTUP"

bool b_alertToAirTag = false;
bool gapScanning     = false;

/* Statio Callback functions */
static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);

static esp_ble_scan_params_t ble_scan_params = {.scan_type          = BLE_SCAN_TYPE_PASSIVE,
                                                .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,
                                                .scan_filter_policy = BLE_SCAN_FILTER_ALLOW_ALL,
                                                .scan_interval      = 0x50,
                                                .scan_window        = 0x30,
                                                .scan_duplicate     = BLE_SCAN_DUPLICATE_DISABLE};

static void esp_gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_SCAN_PARAM_SET_COMPLETE_EVT: {
        uint32_t duration = 30;   // in seconds
        esp_ble_gap_start_scanning(duration);
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        // The scan has either started successfully or failed
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_SCAN_TAG, "Scan start failed, error status = %x",
                     param->scan_start_cmpl.status);
            break;
        }
        ESP_LOGI(GAP_SCAN_TAG, "Scan started successfully");
        gapScanning = true;
        break;

    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        // The scan has aquired results
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        switch (scan_result->scan_rst.search_evt) {
        case ESP_GAP_SEARCH_INQ_RES_EVT:
            // Compare the current packet to what we expect to get
            if ((scan_result->scan_rst.ble_adv[0] == 0x1E) &&
                (scan_result->scan_rst.ble_adv[1] == 0xFF) &&
                (scan_result->scan_rst.ble_adv[2] == 0x4C) &&
                (scan_result->scan_rst.ble_adv[3] == 0x00) &&
                (scan_result->scan_rst.ble_adv[4] == 0x12) &&
                (scan_result->scan_rst.ble_adv[5] == 0x19) &&
                (scan_result->scan_rst.ble_adv[6] == 0x10)) {
                ESP_LOGI(GAP_SCAN_TAG, "AirTag Detected!");
                b_alertToAirTag = true;

                //TODO: Count the number of unique nearby airtags
            }

#if PRINT_OUT_OTHER_DEVICES
            if (scan_result->scan_rst.adv_data_len > 0) {
                ESP_LOGI(GAP_SCAN_TAG, "adv data:");
                esp_log_buffer_hex(GAP_SCAN_TAG, &scan_result->scan_rst.ble_adv[0],
                                   scan_result->scan_rst.adv_data_len);
            }
#endif
            break;

        case ESP_GAP_SEARCH_INQ_CMPL_EVT:
            break;
        default:
            break;
        }
    } break;

    case ESP_GAP_BLE_SCAN_STOP_COMPLETE_EVT:
        // The scan has either stopped successfully or failed
        if (param->scan_stop_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            ESP_LOGE(GAP_SCAN_TAG, "Scan stop failed, error status = %x",
                     param->scan_stop_cmpl.status);
            break;
        }
        ESP_LOGI(GAP_SCAN_TAG, "Scan stopped successfully");
        gapScanning = false;
        break;

    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GAP_SCAN_TAG,
                 "update connection params status = %d, min_int = %d, max_int = %d,conn_int = "
                 "%d,latency = %d, timeout = %d",
                 param->update_conn_params.status, param->update_conn_params.min_int,
                 param->update_conn_params.max_int, param->update_conn_params.conn_int,
                 param->update_conn_params.latency, param->update_conn_params.timeout);
        break;

    default:
        ESP_LOGI(GAP_SCAN_TAG, "Unhandled event in callback: %d", (int)event);
    }
}

void ESP_Init()
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Release the memory for classic BT since we will only be using BLE
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret                               = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "%s initialize coontroller failed: %s\n", __func__,
                 esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "%s init bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "%s enable bluetooth failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }

    //register the  callback function to the gap module
    ret = esp_ble_gap_register_callback(esp_gap_cb);
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "%s gap register failed, error code = %x\n", __func__, ret);
        return;
    }

    ret = esp_ble_gap_set_scan_params(&ble_scan_params);
    if (ret) {
        ESP_LOGE(STARTUP_TAG, "set scan params error, error code = %x", ret);
    }
}

void app_main()
{
    ESP_Init();

    initAlert();
    ESP_LOGI("Main", "Starting delay");
    // Wait for GAP scanning to stop
    vTaskDelay(10 / portTICK_PERIOD_MS);
    ESP_LOGI("MAIN", "Entering Loop");
    do {
        if (b_alertToAirTag) {
            if (gapScanning)
                esp_ble_gap_stop_scanning();

            while (gapScanning) {
                ESP_LOGI("MAIN", "Waiting for scanning to stop");
                vTaskDelay(1);
            }
            alertToAirTag();
            ESP_LOGI("MAIN", "Found AirTag?:%d", b_alertToAirTag);
        }
        vTaskDelay(1);

    } while (gapScanning);

    ESP_LOGI("MAIN", "Entering Sleep...");
    enterSleep();
}