/**
 * main.c
 *
 */

#include <asf.h>
#include <math.h>
#include <drivers/stdio_usart/stdio_usart.h>
#include <drivers/led/led.h>
#include <drivers/pir/pir.h>
#include <drivers/lora/lora.h>
#include <drivers/lora/lora_commands.h>
#include <drivers/grideye/grideye.h>
#include <drivers/people_counting/people_counting.h>
#include <drivers/stdio_usart/logging.h>

static uint32_t inactivity_counter = 0;

void pir_on_wake(void);
static void init_standby(void);
static void sleep_device(void);

/**
 * @brief      Runs when the PIR detects motion
 */
void pir_on_wake(void) {
	// TODO: Implement what happens when the PIR sends an interrupt
	inactivity_counter = 0;
}

static void init_standby(void) {
	struct system_standby_config standby_conf;
	system_standby_get_config_defaults(&standby_conf);
	system_standby_set_config(&standby_conf);
}

static void sleep_device(void) {
	LOG_LINE("MCU Sleeping");
	system_set_sleepmode(SYSTEM_SLEEPMODE_STANDBY);
	system_sleep();
}

static void init_and_join_lora(void) {
	bool joined = false;
	while (!joined) {
		lora_init();
		led_set_state(true);
		delay_ms(500);
		led_set_state(false);
		delay_ms(500);
		led_set_state(true);
		joined = lora_join_otaa();
		led_set_state(false);
	}
}

int main (void)
{
	frame_elem_t grideye_frame[GE_FRAME_SIZE];
	system_init();
	delay_init();
	init_standby();
	led_init();
	stdio_init();
	grideye_init();
	pc_init();
	pir_init(pir_on_wake);
	init_and_join_lora();

	lora_sleep();

	double in_count = 0;
	double out_count = 0;
	double period_in_count = 0;
	double period_out_count = 0;

	while(1) {
		if (inactivity_counter == 50) {
			// Each grideye cycle is ~100ms (since it claims 10FPS), so each tick of the
			//  inactivity counter is assumed to be 100ms.
			pir_disable_interrupt();

			// Send count via Lora
			led_set_state(true);
			if (lora_wake()) {
				if (!lora_join_abp()) {
					init_and_join_lora();
				}
				period_in_count = ceil(period_in_count);
				period_out_count = ceil(period_out_count);
				lora_send_count(period_in_count, period_out_count);
				lora_sleep();
				period_in_count = 0;
				period_out_count = 0;
			}
			led_set_state(false);

			// Sleep MCU + Grideye
			ge_set_mode(GE_MODE_SLEEP);
			pir_enable_interrupt();

			sleep_device();

			// Device is awake here
			led_set_state(true);
			pir_disable_interrupt();
			ge_set_mode(GE_MODE_NORMAL);
			pir_enable_interrupt();
			delay_ms(500);
			led_set_state(false);
		}
		if (!ge_is_sleeping()) {
			pir_disable_interrupt();
			ge_get_frame(grideye_frame);
			pc_new_frame(grideye_frame);
			pir_enable_interrupt();
			in_count = pc_get_in_count();
			out_count = pc_get_out_count();
			period_in_count = ceil(period_in_count);
			period_out_count = ceil(period_out_count);

			if (in_count > 0.0 || out_count > 0.0) {
				period_in_count += in_count;
				period_out_count += out_count;
				if (((double) ((int) period_in_count)) < period_in_count && ((double) ((int) period_out_count)) < period_out_count) {
					LOG_LINE("D(%d.5,%d.5)", (int) period_in_count, (int) period_out_count);
				} else if (((double) ((int) period_in_count)) < period_in_count) {
					LOG_LINE("D(%d.5,%d)", (int) period_in_count, (int) period_out_count);
				} else if (((double) ((int) period_out_count)) < period_out_count) {
					LOG_LINE("D(%d,%d.5)", (int) period_in_count, (int) period_out_count);
				} else {
					LOG_LINE("D(%d,%d)", (int) period_in_count, (int) period_out_count);
				}

				// Reset inactivity counter
				inactivity_counter = 0;
			} else {
				// Increase inactivity counter
				inactivity_counter++;
			}
		}
	}
}
