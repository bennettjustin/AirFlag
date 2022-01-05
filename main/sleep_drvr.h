#ifndef __SLEEP_DRVR_H__
#define __SLEEP_DRVR_H__

void enterSleep(void);
void exitSleep(esp_gap_ble_cb_t esp_gap_cb, esp_ble_scan_params_t *ble_scan_params);

#endif