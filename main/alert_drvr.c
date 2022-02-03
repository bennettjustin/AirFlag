#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include <stdint.h>

#include "driver/gpio.h"
#include "driver/timer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "hal/timer_types.h"

#include "alert_drvr.h"
#include "config.h"

void toggleGPIO(int8_t pin)
{
    if (pin < 0)
        return;

    uint32_t current_value = gpio_get_level(pin) & 0x01;
    gpio_set_level(pin, (~current_value) & 0x01);
}

static uint32_t ledCounter  = 0;
static uint32_t buzzCounter = 0;

static bool ledRunning  = false;
static bool buzzRunning = false;

static void IRAM_ATTR isr_buzzerCallback(uint32_t *args)
{

    toggleGPIO(BUZZER_PIN);

    // If the counter has reached zero
    if (--(*args) <= 0) {
        // Disable timer callback
        timer_pause(BUZZ_TIMER);
        timer_isr_callback_remove(BUZZ_TIMER);

        // Set the buzzer pin to 0
        if (BUZZER_PIN >= 0)
            GPIO_OUTPUT_SET(BUZZER_PIN, 0);

        buzzRunning = false;
    }
}

static void IRAM_ATTR isr_LEDCallback(uint32_t *args)
{
    toggleGPIO(RED_LED_PIN);

    // If the counter has reached zero
    if (--(*args) <= 0) {
        // Disable timer callback
        timer_pause(LED_TIMER);
        timer_isr_callback_remove(LED_TIMER);

        // Set the buzzer pin to 0
        if (RED_LED_PIN >= 0)
            GPIO_OUTPUT_SET(RED_LED_PIN, 0);

        ledRunning = false;
    }
}

void initAlert()
{
    // Configure Buzzer pin if enabled
#if BUZZER_PIN >= 0
    gpio_config_t buzzerConfig = {.mode         = GPIO_MODE_INPUT_OUTPUT,
                                  .pin_bit_mask = ((uint64_t)1) << BUZZER_PIN,
                                  .intr_type    = GPIO_INTR_DISABLE,
                                  .pull_down_en = 0,
                                  .pull_up_en   = 0};
    ESP_ERROR_CHECK(gpio_config(&buzzerConfig));
    GPIO_OUTPUT_SET(BUZZER_PIN, 0);
#endif

    // Configure Red LED pin if enabled
#if RED_LED_PIN >= 0
    gpio_config_t redLEDConfig = {.mode         = GPIO_MODE_INPUT_OUTPUT,
                                  .pin_bit_mask = ((uint64_t)1) << RED_LED_PIN,
                                  .intr_type    = GPIO_INTR_DISABLE,
                                  .pull_down_en = 0,
                                  .pull_up_en   = 0};
    ESP_ERROR_CHECK(gpio_config(&redLEDConfig));
    GPIO_OUTPUT_SET(RED_LED_PIN, 0);
#endif

    // Configure Green LED if enabled
#if GREEN_LED_PIN >= 0
    gpio_config_t ioConfig = {.mode         = GPIO_MODE_OUTPUT,
                              .pin_bit_mask = ((uint64_t)1) << GREEN_LED_PIN,
                              .intr_type    = GPIO_INTR_DISABLE,
                              .pull_down_en = 0,
                              .pull_up_en   = 0};
    ESP_ERROR_CHECK(gpio_config(&ioConfig));
    GPIO_OUTPUT_SET(GREEN_LED_PIN, 0);
#endif
}

// duration in ms
void buzzAlert(uint32_t dur)
{
    if (BUZZER_PIN < 0)
        return;

    // Buzzer needs a 2048Hz Square wave
    // Set a timer to trigger every 250us
    timer_config_t timerConfig = {
        .divider     = APB_CLK_FREQ / TMR_RESOLUTION_HZ,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en  = TIMER_PAUSE,
        .alarm_en    = TIMER_ALARM_EN,
        .auto_reload = true,

    };

    // Initialize the timer to 0 with config
    timer_init(BUZZ_TIMER, &timerConfig);
    timer_set_counter_value(BUZZ_TIMER, 0);

    // Set alarm value and enable interrupt
    uint32_t halfPeriod = 333 / 2;
    timer_set_alarm_value(BUZZ_TIMER, halfPeriod);
    //timer_set_alarm_value(BUZZ_TIMER, 500000);
    timer_enable_intr(BUZZ_TIMER);

    // Set the intr callback
    buzzCounter = dur * (1000 / halfPeriod);
    buzzRunning = true;
    timer_isr_callback_add(BUZZ_TIMER, isr_buzzerCallback, &buzzCounter, 0);
    timer_start(BUZZ_TIMER);
    ESP_LOGI("TMR", "Buzzer timer started");
}

// duration in ms
void ledAlert(uint32_t dur)
{
    if (RED_LED_PIN < 0)
        return;

    // Set a timer to trigger every 500ms
    timer_config_t timerConfig = {
        .divider     = APB_CLK_FREQ / TMR_RESOLUTION_HZ,
        .counter_dir = TIMER_COUNT_UP,
        .counter_en  = TIMER_PAUSE,
        .alarm_en    = TIMER_ALARM_EN,
        .auto_reload = true,

    };

    // Initialize the timer to 0 with config
    timer_init(LED_TIMER, &timerConfig);
    timer_set_counter_value(LED_TIMER, 0);

    // Set alarm value and enable interrupt
    timer_set_alarm_value(LED_TIMER, 500000);
    timer_enable_intr(LED_TIMER);

    // Set the intr callback
    ledCounter = dur / 500;
    ledRunning = true;
    timer_isr_callback_add(LED_TIMER, isr_LEDCallback, &ledCounter, 0);
    timer_start(LED_TIMER);
    ESP_LOGI("TMR", "LED Timer started");
}

void alertToAirTag()
{

    ledAlert(2000);
    buzzAlert(500);

    while (buzzRunning) {
        vTaskDelay(1);
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
    buzzAlert(500);

    while (ledRunning || buzzRunning) {
        vTaskDelay(1);
    }
    // // The beeps (~30 second total time)
    // for (int i = 0; i < 15; i++) {

    //     const uint32_t one_sec = 1000;
    //     alerting               = true;
    //
    //     vTaskDelay(one_sec / portTICK_PERIOD_MS);     // Default tick length is 15ms
    // }
}

void alertToClear() {}