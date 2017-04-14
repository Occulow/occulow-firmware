/*
 * lora.c
 *
 * Created: 3/24/2017 12:08:18 PM
 *  Author: mazacar8
 */

#include <drivers/lora/lora.h>

static struct usart_module lora_usart_module;
static uint8_t rx_buffer[LORA_RX_BUFFER_SIZE];
static uint8_t tx_buffer[LORA_TX_BUFFER_SIZE];

static const char FACTORY_RESET_CMD[] = "sys factoryRESET\r\n";
static const char SET_APPEUI_CMD[] = "mac set appeui %s\r\n";
static const char SET_DEVEUI_CMD[] = "mac set deveui %s\r\n";
static const char SET_APPKEY_CMD[] = "mac set appkey %s\r\n";
static const char SET_DATARATE_CMD[] = "mac set dr %s\r\n";
static const char SET_PWRIDX_CMD[] = "mac set pwridx %s\r\n";
static const char SET_ADR_CMD[] = "mac set adr %s\r\n";
static const char SET_DEVADDR_CMD[] = "mac set devaddr %s\r\n";
static const char SET_PWR_CMD[] = "radio set pwr %s\r\n";
static const char SET_PRLEN_CMD[] = "radio set prlen %s\r\n";
static const char SAVE_CMD[] = "mac save\r\n";
static const char SET_CHANNEL_STATUS_CMD[] = "mac set ch status %d %s\r\n";
static const char JOIN_OTAA_CMD[] = "mac join otaa\r\n";
static const char JOIN_ABP_CMD[] = "mac join abp\r\n";
static const char SEND_UNCONF_CMD[] = "mac tx uncnf 1 %s\r\n";

static void init_power_pins(void);
static void init_usart(void);
static void init_reset(void);
static void setup_channels(void);
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

static void init_power_pins() {
	struct port_config power_gpio_config;
	port_get_config_defaults(&power_gpio_config);
	power_gpio_config.direction = PORT_PIN_DIR_OUTPUT;
	power_gpio_config.input_pull = PORT_PIN_PULL_NONE;

	port_pin_set_config(LORA_POWER_PIN, &power_gpio_config);
	// Enable power
	port_pin_set_output_level(LORA_POWER_PIN, false);
}

static void init_usart(void) {
	struct usart_config lora_usart_config;
	usart_get_config_defaults(&lora_usart_config);
	lora_usart_config.mux_setting = LORA_USART_MUX_SETTING;
	lora_usart_config.pinmux_pad0 = LORA_SERCOM_PINMUX_PAD0;
	lora_usart_config.pinmux_pad1 = LORA_SERCOM_PINMUX_PAD1;
	lora_usart_config.pinmux_pad2 = LORA_SERCOM_PINMUX_PAD2;
	lora_usart_config.pinmux_pad3 = LORA_SERCOM_PINMUX_PAD3;
	lora_usart_config.baudrate = LORA_USART_BAUD;


	while (usart_init(&lora_usart_module, SERCOM1, &lora_usart_config) != STATUS_OK){
	}
	usart_enable(&lora_usart_module);
}

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
 *
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
 *
 */
void lora_join_otaa() {
	int i,j;
	uint16_t cmd_length;
	bool accepted = false;

	cmd_length = sprintf((char *) tx_buffer, SET_APPEUI_CMD, "0000000000000100");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_DEVEUI_CMD, "1122334455667799");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SET_APPKEY_CMD, "2b7e151628aed2a6abf7156669cf4f3c");
	lora_send_cmd(tx_buffer, cmd_length);
	cmd_length = sprintf((char *) tx_buffer, SAVE_CMD);
	lora_send_cmd(tx_buffer, cmd_length);

	// Loop MAX_JOIN_ATTEMPTS to try and join the network
	for(i = 0; i < MAX_JOIN_ATTEMPTS; i++){
		cmd_length = sprintf((char *) tx_buffer, JOIN_OTAA_CMD);
		lora_send_cmd(tx_buffer,cmd_length);

		// TODO: Check with strcmp
		if(rx_buffer[0] != 'o'){
			// Command not received by RN2903. Needs to be sent again.
			printf("Command Failed. Retrying in 10 s\r\n");
			delay_ms(10000);
			continue;
		}

		// Loop until response from gateway is received
		for(j = 0; j < MAX_STATUS_CHECKS; j++){
			if (read_response()) {
				printf("RX: %s\r\n", rx_buffer);
				// Gateway responded
				if(rx_buffer[0] == 'a'){	// TODO: Checking if 'accepted'.
					accepted = true;
					printf("Join Accepted!\r\n");
					break;
				} else if (j == MAX_STATUS_CHECKS - 1) {
					printf("Join Failed... Aborting\r\n");
					accepted = false;
				} else {
					// Join request sent and denied, so new command must be sent
					printf("Join Failed... Retrying in 10 seconds\r\n");
					accepted = false;
					delay_ms(10000);
					break;
				}
			} else {
				//Wait 5 seconds before checking for the response again
				printf("No response, waiting 5 seconds...\r\n");
				delay_ms(5000);
			}
		}
		if(accepted){
			break;
		}
	}
}

/**
 * @brief      Sends a command to the RN2903
 *
 * @param	   cmd	Command to send
 * @param	   len	Length of command to send
 */
lora_status_t lora_send_cmd(lora_cmd_t cmd, uint16_t len) {
	printf("TX: %s", cmd);
	while(usart_write_buffer_wait(&lora_usart_module, cmd, len) != STATUS_OK);
	if (read_response()) {
		printf("RX: %s\r\n", rx_buffer);
		return OK;
	}
	// TODO: Return something better than OK
	return OK;
}

/**
 * @brief      Reads the response from RN2903
 *
 * @return     true if there was a response, false if there wasn't
 */
static bool read_response() {
	uint16_t buffer_size = 0;
	uint16_t new_char = 0;
	uint8_t err_count = 0;
	enum status_code err;

	// Block and read 2 characters always (all responses are at minimum 2 characters)
	while ((err = usart_read_buffer_wait(&lora_usart_module, (uint8_t *) &rx_buffer, 2)) != STATUS_OK) {
		// printf("Fake error reading buffer: %x\r\n", err);
	}

	// Read remaining characters
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
				printf("Timed out\r\n");
				timed_out = true;
				break;
			}
		}
		if (timed_out) {
			continue;
		}
		while((err = usart_read_wait(&lora_usart_module, &new_char)) != STATUS_OK) {
			printf("Error reading buffer: %x\r\n", err);
			if (++err_count > MAX_READ_ATTEMPTS) {
				printf("Error reading buffer: reached maximum read attempts.\r\n");
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
	uint16_t n = sprintf((char *) counts, "%02X%02X", ingress, egress);
	lora_send_data((uint8_t *) counts, n);
}

static void str_to_hex(uint8_t *output, uint8_t *string, uint16_t length) {
	// Assumes length of output is at least 2*length
	for(uint16_t i=0; i < length; i++) {
		sprintf((char *) &output[i*2], "%02X", string[i]);
	}
}