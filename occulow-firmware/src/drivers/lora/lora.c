/*
 * lora.c
 *
 * Created: 3/24/2017 12:08:18 PM
 *  Author: mazacar8
 */

#include <string.h>
#include <drivers/lora/lora.h>
#include <drivers/lora/lora_commands.h>
#include <drivers/stdio_usart/logging.h>

static struct usart_module lora_usart_module;
static uint8_t rx_buffer[LORA_RX_BUFFER_SIZE];
static uint8_t tx_buffer[LORA_TX_BUFFER_SIZE];

static void init_power_pins(void);
static void init_usart(void);
static void init_reset(void);
static void setup_channels(void);
static bool lora_join(lora_cmd_t join_cmd, uint16_t cmd_length);
static bool read_response(void);
static void str_to_hex(uint8_t *output, uint8_t *string, uint16_t length);


/**
 * @brief      Initializes and saves setup values for RN2903
 *
 */
void lora_init() {
	uint16_t cmd_length;
	init_power_pins();
	init_usart();
	init_reset();
	lora_reset();

	// TODO: Break this up
	cmd_length = sprintf((char *) tx_buffer, FACTORY_RESET_CMD);
	lora_send_cmd(tx_buffer, cmd_length);

	cmd_length = sprintf((char *) tx_buffer, SET_DATARATE_CMD, "0");
	lora_send_cmd(tx_buffer, cmd_length);

	cmd_length = sprintf((char *) tx_buffer, SET_PWRIDX_CMD, "5");
	lora_send_cmd(tx_buffer, cmd_length);

	cmd_length = sprintf((char *) tx_buffer, SET_ADR_CMD, "off");
	lora_send_cmd(tx_buffer, cmd_length);

	setup_channels();

	cmd_length = sprintf((char *) tx_buffer, SET_DEVADDR_CMD, "00000000");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_PWR_CMD, "20");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_PRLEN_CMD, "8");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SAVE_CMD);
	lora_send_cmd(tx_buffer, cmd_length);
}

/**
 * @brief      Initializes the power for the LoRa module and turns it on
 */
static void init_power_pins() {
	struct port_config power_gpio_config;
	port_get_config_defaults(&power_gpio_config);
	power_gpio_config.direction = PORT_PIN_DIR_OUTPUT;
	power_gpio_config.input_pull = PORT_PIN_PULL_NONE;

	port_pin_set_config(LORA_POWER_PIN, &power_gpio_config);
	// Enable power
	port_pin_set_output_level(LORA_POWER_PIN, true);
	delay_ms(500);
	port_pin_set_output_level(LORA_POWER_PIN, false);
}

/**
 * @brief      Initializes the UART SERCOM for LoRa
 */
static void init_usart(void) {
	struct usart_config lora_usart_config;
	usart_get_config_defaults(&lora_usart_config);
	lora_usart_config.mux_setting = LORA_USART_MUX_SETTING;
	lora_usart_config.pinmux_pad0 = LORA_SERCOM_PINMUX_PAD0;
	lora_usart_config.pinmux_pad1 = LORA_SERCOM_PINMUX_PAD1;
	lora_usart_config.pinmux_pad2 = LORA_SERCOM_PINMUX_PAD2;
	lora_usart_config.pinmux_pad3 = LORA_SERCOM_PINMUX_PAD3;
	lora_usart_config.run_in_standby = true;
	lora_usart_config.baudrate = LORA_USART_BAUD;


	while (usart_init(&lora_usart_module, SERCOM1, &lora_usart_config) != STATUS_OK){
	}
	usart_enable(&lora_usart_module);
}

/**
 * @brief      Initializes the reset_n pin for LoRa module
 */
static void init_reset(void) {
	struct port_config reset_config;
	port_get_config_defaults(&reset_config);
	reset_config.direction = PORT_PIN_DIR_OUTPUT;
	reset_config.input_pull = PORT_PIN_PULL_NONE;
	reset_config.powersave = false;
	port_pin_set_config(LORA_RESET_PIN, &reset_config);
}

/**
 * @brief      Resets the RN2903
 *
 */
void lora_reset(){
	port_pin_set_output_level(LORA_RESET_PIN, false);
	port_pin_set_output_level(LORA_RESET_PIN, true);
}

/**
 * @brief      Switches off all but channels 0-7 for Lora transmit
 */
void setup_channels() {
	uint16_t cmd_length;
	// TODO: Extract magic numbers
	for (int i = 0; i < 72; i++) {
		if (i < 8){
			cmd_length = (uint16_t) sprintf((char *) tx_buffer, SET_CHANNEL_STATUS_CMD, i, "on");
			lora_send_cmd(tx_buffer, cmd_length);
		} else {
			cmd_length = (uint16_t) sprintf((char *) tx_buffer, SET_CHANNEL_STATUS_CMD, i, "off");
			lora_send_cmd(tx_buffer, cmd_length);
		}
	}
}

/**
 * @brief      Attempts to join the Lora network a maximum of 5 times
 */
bool lora_join_otaa() {
	uint16_t cmd_length;

	// Set keys and parameters
	cmd_length = sprintf((char *) tx_buffer, SET_APPEUI_CMD, "0000000000000100");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_DEVEUI_CMD, "1122334455667799");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_APPKEY_CMD, "2b7e151628aed2a6abf7156669cf4f3c");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_DEVADDR_CMD, "00000000");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_NWK_SKEY, "00000000000000000000000000000000");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_APP_SKEY, "00000000000000000000000000000000");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SAVE_CMD);
	lora_send_cmd(tx_buffer, cmd_length);

	// Join Network
	cmd_length = sprintf((char *) tx_buffer, JOIN_OTAA_CMD);
	return lora_join(tx_buffer, cmd_length);
}

bool lora_join_abp(void) {
	uint16_t cmd_length = 0;
	cmd_length = sprintf((char *) tx_buffer, JOIN_ABP_CMD);
	return lora_join(tx_buffer, cmd_length);
}

static bool lora_join(lora_cmd_t join_cmd, uint16_t cmd_length) {
	// Loop MAX_JOIN_ATTEMPTS to try and join the network
	for(uint16_t i = 0; i < MAX_JOIN_ATTEMPTS; i++){
		if (lora_send_cmd(join_cmd, cmd_length) == SEND_FAIL) {
			LOG_LINE("Unable to send join command: %s", join_cmd);
			continue;
		}

		if(strncmp(rx_buffer, LORA_OK, sizeof(LORA_OK) - 1) != 0){
			// Command not received by RN2903. Needs to be sent again.
			LOG_LINE("Command Failed. Retrying in %d millis", LORA_REJOIN_DELAY);
			delay_ms(LORA_REJOIN_DELAY);
		} else {
			// Read second repsonse
			if (read_response()) {
				LOG_LINE("RX: %s", rx_buffer);
				// Gateway responded
				if(strncmp(rx_buffer, LORA_ACCEPTED, sizeof(LORA_ACCEPTED) - 1) == 0){
					LOG_LINE("Join Accepted!");
					// Save keys
					cmd_length = sprintf((char *) tx_buffer, SAVE_CMD);
					lora_send_cmd(tx_buffer, cmd_length);
					return true;
				} else {
					// Join request sent and denied, so new command must be sent
					LOG_LINE("Join Failed... Retrying in %d millis", LORA_REJOIN_DELAY);
					delay_ms(LORA_REJOIN_DELAY);
				}
			} else {
				//Wait before checking for the response again
				LOG_LINE("No response, waiting %d seconds...", LORA_REJOIN_DELAY);
				delay_ms(LORA_REJOIN_DELAY);
				// TODO: Decide if we should exit early here or retry
			}
		}
	}
	return false;
}

/**
 * @brief      Sleeps the lora module indefinitely
 */
void lora_sleep(void) {
	// Sleep "indefinitely": 24.86 days (INT_MAX ms)
	uint16_t cmd_length = sprintf((char *) tx_buffer, SLEEP_CMD, 2147483647);
	LOG_LINE("TX: %s", tx_buffer);
	while(usart_write_buffer_wait(&lora_usart_module, tx_buffer, cmd_length) != STATUS_OK);
}

/**
 * @brief      Wakes up the lora module, assuming that it is asleep
 */
bool lora_wake(void) {
	struct usart_config lora_wake_config;

	// Configure baud rate to be 1/2 of the normal config (so sending 0x0 is a
	// break condition)
	usart_disable(&lora_usart_module);
	usart_get_config_defaults(&lora_wake_config);
	lora_wake_config.mux_setting = LORA_USART_MUX_SETTING;
	lora_wake_config.pinmux_pad0 = LORA_SERCOM_PINMUX_PAD0;
	lora_wake_config.pinmux_pad1 = LORA_SERCOM_PINMUX_PAD1;
	lora_wake_config.pinmux_pad2 = LORA_SERCOM_PINMUX_PAD2;
	lora_wake_config.pinmux_pad3 = LORA_SERCOM_PINMUX_PAD3;
	// Anything less than LORA_USART_BAUD/2 will work
	lora_wake_config.baudrate = LORA_USART_BAUD/2;
	while (usart_init(&lora_usart_module, SERCOM1, &lora_wake_config) != STATUS_OK);

	// Signal break condition ("UART_RX pin low for longer than the time to
	// transmit a complete character")
	usart_enable(&lora_usart_module);
	while(usart_write_wait(&lora_usart_module, 0x0) != STATUS_OK);

	// Restore normal config (normal baud rate)
	usart_disable(&lora_usart_module);
	init_usart();

	// Send autobaud detection cmd
	while(usart_write_buffer_wait(&lora_usart_module, AUTO_BAUD_CMD,
		sizeof(AUTO_BAUD_CMD)) != STATUS_OK);

	// Receive "OK" from chip waking up
	if (read_response()) {
		LOG_LINE("Wake: %s", rx_buffer);
	} else {
		return false;
	}

	// HACK: Not sure why the chip also send "in" after "ok" on wake
	if (read_response()) {
		LOG_LINE("Wake: %s", rx_buffer);
	} else {
		return false;
	}
	return true;
}

/**
 * @brief      Sends a command to the RN2903
 *
 * @param	   cmd	Command to send
 * @param	   len	Length of command to send
 */
lora_status_t lora_send_cmd(lora_cmd_t cmd, uint16_t len) {
	LOG_LINE("TX: %s", cmd);
	while(usart_write_buffer_wait(&lora_usart_module, cmd, len) != STATUS_OK);
	if (read_response()) {
		LOG_LINE("RX: %s", rx_buffer);
		return OK;
	}
	// TODO: Return something better than OK
	return FAIL;
}

/**
 * @brief      Reads the response from RN2903
 *
 * @return     true if there was a response, false if there wasn't
 */
static bool read_response() {
	uint16_t buffer_size = 0;
	uint16_t new_char = 0;
	uint16_t err_count = 0;
	enum status_code err;

	// Block and read 2 characters always (all responses are at minimum 2 characters)
	while ((err = usart_read_buffer_wait(&lora_usart_module, (uint8_t *) rx_buffer, 2)) != STATUS_OK) {
		// LOG_LINE("Fake error reading buffer: %x", err);
		if (++err_count > MAX_READ_ATTEMPTS) {
			LOG_LINE("Error reading buffer (0): reached maximum read attempts.");
			return false;
		}
	}

	// Read remaining characters
	err_count = 0;
	buffer_size += 2;
	bool timed_out = false;
	while (new_char != '\n') {
		// TODO: Fix ugliness
		timed_out = false;
		// Wait for module to be ready
		for (uint32_t i = 0; i <= USART_TIMEOUT; i++) {
			if (lora_usart_module.hw->USART.INTFLAG.reg & SERCOM_USART_INTFLAG_RXC) {
				break;
			} else if (i == USART_TIMEOUT) {
				LOG_LINE("Timed out");
				timed_out = true;
				break;
			}
		}
		if (timed_out) {
			if (++err_count > MAX_READ_ATTEMPTS) {
				LOG_LINE("Error reading buffer (1): reached maximum read attempts.");
				return false;
			}
			continue;
		}
		while((err = usart_read_wait(&lora_usart_module, &new_char)) != STATUS_OK) {
			LOG_LINE("Error reading buffer: %x", err);
			if (++err_count > MAX_READ_ATTEMPTS) {
				LOG_LINE("Error reading buffer (2): reached maximum read attempts.");
				return false;
			}
		}
		rx_buffer[buffer_size++] = new_char;
	}
	rx_buffer[buffer_size] = '\0';
	return true;
}

/**
 * @brief      Sends "string" over the Lora Network
 *
 * @param	   string	String to send over the network
 */
void lora_send_data(uint8_t *str, uint16_t length) {
	uint8_t hex_str[LORA_TX_BUFFER_SIZE - sizeof(SEND_UNCONF_CMD)];
	str_to_hex((uint8_t *) hex_str, str, length);

	uint16_t cmd_length = sprintf((char *) tx_buffer, SEND_UNCONF_CMD, hex_str);
	lora_send_cmd(tx_buffer, cmd_length);
}

/**
 * @brief      Sends an integer over the Lora Network
 *
 * @param	   count	Integer to send over the network
 */
void lora_send_count(uint16_t ingress, uint16_t egress){
	uint8_t counts[8];
	sprintf((char *) counts, "%02X%02X", ingress, egress);
	uint16_t cmd_length = sprintf((char *) tx_buffer, SEND_UNCONF_CMD, counts);
	lora_send_cmd(tx_buffer, cmd_length);
	if (read_response()) {
		LOG_LINE("RX2: %s", rx_buffer);
	}
}

/**
 * @brief      Converts a character string in to a hex string
 *
 * @param      output  Output buffer (size >= 2*length + 1)
 * @param      string  Character string to convert
 * @param[in]  length  Length of character string
 */
static void str_to_hex(uint8_t *output, uint8_t *string, uint16_t length) {
	// Assumes length of output is at least 2*length + 1
	for(uint16_t i=0; i < length; i++) {
		sprintf((char *) &output[i*2], "%02X", string[i]);
	}
}
