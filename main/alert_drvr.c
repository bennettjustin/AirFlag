#include "alert_drvr.h"
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/timer.h"
#include "hal/timer_types.h"
#include "esp_err.h"
#include "esp_log.h"

static void IRAM_ATTR isr_buzzerCallback(void *args)
{
    uint16_t *countPtr = (uint16_t)(args);

    toggleGPIO(BUZZER_PIN);

    // If the counter has reached zero
    if(--(*countPtr) == 0)
    {
        // Disable timer callback
        timer_pause(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER);
        timer_isr_callback_remove(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER);

        // Set the buzzer pin to 0
        if (BUZZER_PIN >= 0)
            GPIO_OUTPUT_SET(BUZZER_PIN, 0);

        // Free the counter mem
        free(countPtr);
        ESP_LOGI("ALERT", "Buzzer finished");
    }
    
}
 
void initAlert()
{
    // Configure Buzzer pin if enabled
    if (BUZZER_PIN >= 0){
        gpio_config_t ioConfig = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = ((uint64_t)1) << BUZZER_PIN,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_down_en = 0,
            .pull_up_en = 0
        };
        ESP_ERROR_CHECK(gpio_config(&ioConfig));
    }

    // Configure Red LED pin if enabled
    if (RED_LED >= 0){
        gpio_config_t ioConfig = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = ((uint64_t)1) << RED_LED,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_down_en = 0,
            .pull_up_en = 0
        };
        ESP_ERROR_CHECK(gpio_config(&ioConfig));
    }

    // Configure Green LED if enabled
    if (GREEN_LED >= 0){
        gpio_config_t ioConfig = {
            .mode = GPIO_MODE_OUTPUT,
            .pin_bit_mask = ((uint64_t)1) << GREEN_LED,
            .intr_type = GPIO_INTR_DISABLE,
            .pull_down_en = 0,
            .pull_up_en = 0
        };
        ESP_ERROR_CHECK(gpio_config(&ioConfig));
    }
        
}

void toggleGPIO(int8_t pin)
{
    if(pin < 0)
        return;

    gpio_get_level(pin) ? (GPIO_OUTPUT_SET(pin, 0)) : (GPIO_OUTPUT_SET(pin, 1));
}

// duration in ms
void buzzAlert(uint32_t dur)
{
    if (BUZZER_PIN < 0)
        return;

    // Buzzer needs a 2048Hz Square wave
    // Set a timer to trigger every 250us
    timer_config_t timerConfig = {
        .divider = APB_CLK_FREQ / TIMER_RESOLUTION_HZ,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en = TIMER_PAUSE,
        .alarm_en = TIMER_ALARM_EN,
        .auto_reload = true,
        
    };

    // Initialize the timer to 0 with config
    ESP_ERROR_CHECK(timer_init(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER, &timerConfig));
    ESP_ERROR_CHECK(timer_set_counter_value(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER, 0));

    // Set alarm value and enable interrupt
    ESP_ERROR_CHECK(timer_set_alarm_value(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER, 250));
    ESP_ERROR_CHECK(timer_enable_intr(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER));

    // Set the intr callback
    uint16_t *countPtr = (uint16_t *)malloc(sizeof(uint16_t));
    *countPtr = dur * 4;
    ESP_ERROR_CHECK(timer_isr_callback_add(BUZZER_TMR_GROUP, BUZZER_TMR_TIMER, isr_buzzerCallback, countPtr, 0));

}

void alertToAirTag()
{
    // The beeps (~30 second total time)
    for (int i = 0; i < 15; i++) {
        
        const uint32_t one_sec = 1000;
        buzzAlert(one_sec);
        vTaskDelay(one_sec / portTICK_PERIOD_MS); // Default tick length is 15ms
    }
}
