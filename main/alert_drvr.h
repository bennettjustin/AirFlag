#ifndef ALERT_DRVR
#define ALERT_DRVR

#include <stdint.h>

#define BUZZER_PIN -1
#define RED_LED -1
#define GREEN_LED -1

#define TIMER_RESOLUTION_HZ 1000000     // 1Mhz (1us)

#define BUZZER_TMR_GROUP 0
#define BUZZER_TMR_TIMER 0

void initAlert();
void toggleGPIO(int8_t pin);
void buzzAlert(uint8_t dur);

#endif