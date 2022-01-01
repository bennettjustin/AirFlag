#ifndef SLEEP_DRVR
#define SLEEP_DRVR

//#include "esp_gap_ble_api.h"

#define SLEEP_TIME_US 120000000 // 2 minutes

void enterSleep(void);
void exitSleep(esp_gap_ble_cb_t esp_gap_cb, esp_ble_scan_params_t *ble_scan_params);

#endif