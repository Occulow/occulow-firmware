/*
 * pir.c
 *
 * Created: 4/7/2017 5:04:36 PM
 *  Author: Terence Sun (tsun1215)
 */ 
#include <pir.h>

/**
 * @brief      Initializes the PIR_VCC_CTRL line for power control
 */
void init_power_pins(void);

/**
 * @brief      Initializes the interrupt controller to call on_wake on interrupt
 *
 * @param[in]  on_wake  Callback function to call on interrupt
 */
void init_pir_interrupt(extint_callback_t on_wake);

void pir_init(extint_callback_t on_wake) {
	init_power_pins();
	init_pir_interrupt(on_wake);
}

void init_power_pins(void) {
	// Configure a pinmux for output
	struct system_pinmux_config pinmux_out_cfg;
	system_pinmux_get_config_defaults(&pinmux_out_cfg);
	pinmux_out_cfg.mux_position = SYSTEM_PINMUX_GPIO;
	pinmux_out_cfg.direction = SYSTEM_PINMUX_PIN_DIR_OUTPUT;
	pinmux_out_cfg.input_pull = SYSTEM_PINMUX_PIN_PULL_UP;

	system_pinmux_pin_set_config(PIR_PWR_CTRL, &pinmux_out_cfg);
}

void init_pir_interrupt(extint_callback_t on_wake) {
	struct extint_chan_conf config_extint_chan;
	extint_chan_get_config_defaults(&config_extint_chan);
	
	// Set external interrupt configuration
	config_extint_chan.gpio_pin = PIR_PORT;
	config_extint_chan.gpio_pin_mux = PIR_EIC_PINMUX;
	config_extint_chan.gpio_pin_pull = EXTINT_PULL_UP;
	config_extint_chan.detection_criteria = EXTINT_DETECT_RISING;
	extint_chan_set_config(PIR_EIC_LINE, &config_extint_chan);
	
	// Clear channel until stabilized
	while(extint_chan_is_detected(PIR_EIC_LINE)) {
		extint_chan_clear_detected(PIR_EIC_LINE);
	}
	
	// Register and enable on_wake callback function
	extint_register_callback(on_wake, PIR_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
	extint_chan_enable_callback(PIR_EIC_LINE, EXTINT_CALLBACK_TYPE_DETECT);
}
