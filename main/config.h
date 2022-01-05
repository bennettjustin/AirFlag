#ifndef __CONFIG_H__
#define __CONFIG_H__

// The pins used for alerts. Can be any pin available on the ESP32.
// Set to -1 to disable
#define BUZZER_PIN 2
#define RED_LED    -1
#define GREEN_LED  -1

// How long to alert to a detected AirTag
#define ALERT_LENGTH 60     // In seconds

// How long to sleep in between scans
#define SLEEP_TIME_US 120000000     // 2 minutes

// How long to scan for AirTags
#define GAP_SCAN_DURATION 30     // In seconds

// The maximum number of AirTags to scan for.
#define MAX_AIRTAG_COUNT 10

// Set to 1 to see the advertisment data from all devices in range. Set to 0 to disable
#define PRINT_OUT_OTHER_DEVICES 0

#endif