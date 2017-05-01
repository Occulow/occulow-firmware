/*
 * led.c
 *
 * Created: 4/28/2017 3:51:49 PM
 *  Author: tsun
 */
#include <drivers/led/led.h>


void led_init(void) {
	struct port_config led_gpio_conf;
	port_get_config_defaults(&led_gpio_conf);
	led_gpio_conf.direction = PORT_PIN_DIR_OUTPUT_WTH_READBACK;
	led_gpio_conf.input_pull = PORT_PIN_PULL_NONE;

	port_pin_set_config(LED_PIN, &led_gpio_conf);

	// Default to off
	port_pin_set_output_level(LED_PIN, false);
}
void led_set_state(bool state) {
	port_pin_set_output_level(LED_PIN, state);
}
bool led_get_state(void) {
	return port_pin_get_output_level(LED_PIN);
}