#include "hw_io.h"

#include <string.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "esp_err.h"

#define CHANNEL_COUNT		7
#define PWM_RESOLUTION		LEDC_TIMER_13_BIT
#define MAX_DUTY 		 	8192
#define PWM_FREQUENCY		5000
#define INPUT_PIN			19

typedef enum {
	PWM_A_R, PWM_A_G, PWM_A_B, PWM_B_R, PWM_B_G, PWM_B_B, PWM_H,
} pwm_channel_t;

// PWM values for LED dimmer channels and halogens. Value: 0.0 - 1.0
volatile float pwms[7] = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 };

// enable PWM for Halogens
volatile int en_H = 0;

// enable PWM for LEDs bank A
volatile int en_A = 0;

// enable PWM for LEDs bank B
volatile int en_B = 0;

// enable for relay 1
volatile int en_REL1 = 0;

// enable for relay 2
volatile int en_REL2 = 0;

ledc_channel_config_t ledc_channel[CHANNEL_COUNT];

static void channel_config(void) {

	// config the timer: freq and resolution
	ledc_timer_config_t ledc_timer;
	ledc_timer.duty_resolution = PWM_RESOLUTION; // resolution of PWM duty
	ledc_timer.freq_hz = PWM_FREQUENCY;                      // frequency of PWM signal
	ledc_timer.speed_mode = LEDC_HIGH_SPEED_MODE;   // timer mode
	ledc_timer.timer_num = LEDC_TIMER_0;            // timer index
	ledc_timer.clk_cfg = LEDC_AUTO_CLK;             // Auto select the source clock
	ledc_timer_config(&ledc_timer);

	// config the channels
	ledc_channel[PWM_A_R].gpio_num = 14;
	ledc_channel[PWM_A_R].duty = 0;
	ledc_channel[PWM_A_R].hpoint = 0;
	ledc_channel[PWM_A_R].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_A_R].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_A_R].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_A_R].channel = PWM_A_R;
	ledc_channel_config(&ledc_channel[PWM_A_R]);

	ledc_channel[PWM_A_G].gpio_num = 12;
	ledc_channel[PWM_A_G].duty = 0;
	ledc_channel[PWM_A_G].hpoint = 0;
	ledc_channel[PWM_A_G].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_A_G].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_A_G].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_A_G].channel = PWM_A_G;
	ledc_channel_config(&ledc_channel[PWM_A_G]);

	ledc_channel[PWM_A_B].gpio_num = 13;
	ledc_channel[PWM_A_B].duty = 0;
	ledc_channel[PWM_A_B].hpoint = 0;
	ledc_channel[PWM_A_B].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_A_B].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_A_B].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_A_B].channel = PWM_A_B;
	ledc_channel_config(&ledc_channel[PWM_A_B]);

	ledc_channel[PWM_B_R].gpio_num = 15;
	ledc_channel[PWM_B_R].duty = 0;
	ledc_channel[PWM_B_R].hpoint = 0;
	ledc_channel[PWM_B_R].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_B_R].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_B_R].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_B_R].channel = PWM_B_R;
	ledc_channel_config(&ledc_channel[PWM_B_R]);

	ledc_channel[PWM_B_G].gpio_num = 2;
	ledc_channel[PWM_B_G].duty = 0;
	ledc_channel[PWM_B_G].hpoint = 0;
	ledc_channel[PWM_B_G].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_B_G].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_B_G].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_B_G].channel = PWM_B_G;
	ledc_channel_config(&ledc_channel[PWM_B_G]);

	ledc_channel[PWM_B_B].gpio_num = 4;
	ledc_channel[PWM_B_B].duty = 0;
	ledc_channel[PWM_B_B].hpoint = 0;
	ledc_channel[PWM_B_B].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_B_B].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_B_B].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_B_B].channel = PWM_B_B;
	ledc_channel_config(&ledc_channel[PWM_B_B]);

	ledc_channel[PWM_H].gpio_num = 16;
	ledc_channel[PWM_H].duty = 0;
	ledc_channel[PWM_H].hpoint = 0;
	ledc_channel[PWM_H].intr_type = LEDC_INTR_DISABLE;
	ledc_channel[PWM_H].speed_mode = LEDC_HIGH_SPEED_MODE;
	ledc_channel[PWM_H].timer_sel = LEDC_TIMER_0;
	ledc_channel[PWM_H].channel = PWM_H;
	ledc_channel_config(&ledc_channel[PWM_H]);

}

static void inputs_config(void) {

	gpio_config_t gpio_config_struct;
	gpio_config_struct.intr_type = GPIO_INTR_DISABLE;
	gpio_config_struct.mode = GPIO_MODE_INPUT;
	gpio_config_struct.pin_bit_mask = (1 << INPUT_PIN);
	gpio_config_struct.pull_down_en = 0;
	gpio_config_struct.pull_up_en = 1;
	gpio_config(&gpio_config_struct);

}

static void fade_to_val_nowait(pwm_channel_t channel, float duty, int time_ms) {

	if (duty > 1.0)
		duty = 1.0;
	if (duty < 0.0)
		duty = 0.0;

	int dutyval = (int) ((float) MAX_DUTY * duty);

	if (time_ms > 0) {
		ledc_set_fade_with_time(ledc_channel[channel].speed_mode, ledc_channel[channel].channel, dutyval, time_ms);
		ledc_fade_start(ledc_channel[channel].speed_mode, ledc_channel[channel].channel, LEDC_FADE_NO_WAIT);
	} else {
		ledc_set_duty(ledc_channel[channel].speed_mode, ledc_channel[channel].channel, MAX_DUTY);
		ledc_update_duty(ledc_channel[channel].speed_mode, ledc_channel[channel].channel);
	}
}

void hw_io_task(void *pvParameters) {

	printf("Hardware IO task started.\r\n");

	// configure the 7 pwm channels
	channel_config();

	//configure GPIO inputs
	inputs_config();

	// Initialize fade service.
	ledc_fade_func_install(0);

	int input_val_old = 0;
	int input_val = 0;
	const int DELAY = 500;

	while (1) {

		input_val = gpio_get_level(INPUT_PIN);
		if (input_val != input_val_old) {

			// update the old val
			input_val_old = input_val;

			//toggle the enable flags based on Halogens
			if (en_H == 0)
			{
				en_H = 1;
				en_A = 1;
				en_B = 1;
			}
			else
			{
				en_H = 0;
				en_A = 0;
				en_B = 0;
			}
		}

		if (en_A == 1) {
			fade_to_val_nowait(PWM_A_R, pwms[PWM_A_R], DELAY);
			fade_to_val_nowait(PWM_A_G, pwms[PWM_A_G], DELAY);
			fade_to_val_nowait(PWM_A_B, pwms[PWM_A_B], DELAY);

		} else {
			fade_to_val_nowait(PWM_A_R, 0, DELAY);
			fade_to_val_nowait(PWM_A_G, 0, DELAY);
			fade_to_val_nowait(PWM_A_B, 0, DELAY);
		}

		if (en_B == 1) {
			fade_to_val_nowait(PWM_B_R, pwms[PWM_B_R], DELAY);
			fade_to_val_nowait(PWM_B_G, pwms[PWM_B_G], DELAY);
			fade_to_val_nowait(PWM_B_B, pwms[PWM_B_B], DELAY);

		} else {
			fade_to_val_nowait(PWM_B_R, 0, DELAY);
			fade_to_val_nowait(PWM_B_G, 0, DELAY);
			fade_to_val_nowait(PWM_B_B, 0, DELAY);
		}

		if (en_A == 1) {
			fade_to_val_nowait(PWM_H, pwms[PWM_H], DELAY);

		} else {
			fade_to_val_nowait(PWM_H, 0, DELAY);
		}

		vTaskDelay(100);
	}
}

