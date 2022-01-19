#ifndef __ALERT_DRVR_H__
#define __ALERT_DRVR_H__

#include <stdint.h>

/*********************
 * Defined in config.h -- DO NOT CHANGE HERE
#define BUZZER_PIN   2
#define RED_LED      -1
#define GREEN_LED    -1
#define ALERT_LENGTH 60     //in seconds
**********************/

#define TMR_RESOLUTION_HZ 1000000     // 1Mhz (1us)

#define BUZZER_TMR_GROUP 0
#define BUZZER_TMR_TIMER 0
#define BUZZ_TIMER       BUZZER_TMR_GROUP, BUZZER_TMR_TIMER

#define LED_TMR_GROUP 0
#define LED_TMR_TIMER 1
#define LED_TIMER     LED_TMR_GROUP, LED_TMR_TIMER

bool alerting;

void initAlert();
void toggleGPIO(int8_t pin);
void buzzAlert(uint32_t dur);
void ledAlert(uint32_t dur);
void alertToAirTag();

#endif