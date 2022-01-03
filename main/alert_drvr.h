#ifndef ALERT_DRVR
#define ALERT_DRVR

#include <stdint.h>

#define BUZZER_PIN 2
#define RED_LED    -1
#define GREEN_LED  -1

#define BUZZ_TMR_RESOLUTION_HZ 1000000     // 1Mhz (1us)
#define LED_TMR_RESULUTION_HZ  1000        // 1kHz (1ms)

#define BUZZER_TMR_GROUP 0
#define BUZZER_TMR_TIMER 0
#define BUZZ_TIMER       BUZZER_TMR_GROUP, BUZZER_TMR_TIMER

#define LED_TMR_GROUP 0
#define LED_TMR_TIMER 1
#define LED_TIMER     LED_TMR_GROUP, LED_TMR_TIMER

#define ALERT_LENGTH 60     //in seconds

void initAlert();
void toggleGPIO(int8_t pin);
void buzzAlert(uint32_t dur);
void ledAlert(uint32_t dur);
void alertToAirTag();

#endif