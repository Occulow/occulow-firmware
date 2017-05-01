/**
 * main.c
 *
 */

#include <asf.h>
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
	LOG_LINE("PIR Wake!");
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

int main (void)
{
	frame_elem_t grideye_frame[GE_FRAME_SIZE];
	system_init();
	delay_init();
	init_standby();
	led_init();
	stdio_init();
	lora_init();
	//grideye_init();
	//pc_init();
	//pir_init(pir_on_wake);

	lora_join_otaa();
	lora_sleep();

	double in_count = 0;
	double out_count = 0;
	double period_in_count = 0;
	double period_out_count = 0;

	while(1) {
		delay_ms(5000);

		led_set_state(true);
		lora_wake();
		//lora_send_cmd(MAC_RESUME, sizeof(MAC_RESUME));
		lora_join_abp();
		lora_send_count(1,1);
		lora_sleep();
		led_set_state(false);

		//ge_set_mode(GE_MODE_SLEEP);
		//pir_enable_interrupt();
		//sleep_device();
		//pir_disable_interrupt();
		//ge_set_mode(GE_MODE_NORMAL);
		/*if (inactivity_counter == 50) {
			// Each grideye cycle is ~100ms (since it claims 10FPS), so each tick of the
			//  inactivity counter is assumed to be 100ms.
			inactivity_counter = 0;
			led_set_state(true);
			lora_wake();
			lora_join_abp();
			lora_send_count(period_in_count, period_out_count);
			lora_sleep();
			led_set_state(false);
			period_in_count = 0;
			period_out_count = 0;
			ge_set_mode(GE_MODE_SLEEP);
			//port_pin_set_output_level(GE_PWR_PIN, true);
			pir_enable_interrupt();
			sleep_device();
			pir_disable_interrupt();
			//port_pin_set_output_level(GE_PWR_PIN, false);
			ge_set_mode(GE_MODE_NORMAL);
			// pc_flush_buffer();
		}
		if (!ge_is_sleeping()) {
			ge_get_frame(grideye_frame);
			pc_new_frame(grideye_frame);
			in_count = pc_get_in_count();
			out_count = pc_get_out_count();

			if (in_count > 0.0 || out_count > 0.0) {
				if (((double) ((int) in_count)) < in_count && ((double) ((int) out_count)) < out_count) {
					LOG_LINE("D(%d.5,%d.5)", (int) in_count, (int) out_count);
				} else if (((double) ((int) in_count)) < in_count) {
					LOG_LINE("D(%d.5,%d)", (int) in_count, (int) out_count);
				} else if (((double) ((int) out_count)) < out_count) {
					LOG_LINE("D(%d,%d.5)", (int) in_count, (int) out_count);
				} else {
					LOG_LINE("D(%d,%d)", (int) in_count, (int) out_count);
				}
				period_in_count += in_count;
				period_out_count += out_count;

				// Reset inactivity counter
				inactivity_counter = 0;
			} else {
				// Increase inactivity counter
				inactivity_counter++;
			}
		}*/
	}
}
