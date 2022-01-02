#ifndef ALERT_DRVR
#define ALERT_DRVR

#include <stdint.h>

#define BUZZER_PIN 2
#define RED_LED    -1
#define GREEN_LED  -1

#define TIMER_RESOLUTION_HZ 1000000   // 1Mhz (1us)

#define BUZZER_TMR_GROUP 0
#define BUZZER_TMR_TIMER 0

#define ALERT_TMR_GROUP 0
#define ALERT_TMR_TIMER 1
#define ALERT_TMR()     (ALERT_TMR_GROUP), (ALERT_TMR_TIMER)

#define ALERT_LENGTH 60   //in seconds

void initAlert();
void toggleGPIO(int8_t pin);
void buzzAlert(uint32_t dur);
void alertToAirTag();

#endif