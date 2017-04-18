/*
* grideye.c
*
* Created: 3/1/2017 8:28:44 PM
*  Author: Terence Sun (tsun1215)
*/

#include <asf.h>
#include <drivers/grideye/grideye.h>

#define GE_BUFFER_DATA_LENGTH 128
static uint8_t ge_write_buffer[GE_BUFFER_DATA_LENGTH];
static uint8_t ge_read_buffer[GE_BUFFER_DATA_LENGTH];
static uint8_t ge_mode = GE_MODE_NORMAL;

#define GE_SLAVE_ADDRESS 0x68
#define GE_REG_THERM_LSB 0x0E
#define GE_REG_THERM_MSB 0x0F
#define GE_REG_PIXEL_BASE 0x80
#define GE_REG_STATE 0x00
#define GE_REG_RESET 0x01

#define GE_CMD_INITIAL_RESET 0x3F
#define GE_CMD_FLAG_RESET 0x30

/* Number of times to try to send packet if failed. */
#define TIMEOUT 1000

/* Init software module. */
struct i2c_master_module i2c_master_instance;

static uint8_t read_byte(uint8_t addr);
static void write_byte(uint8_t addr, uint8_t *data, uint8_t length);
static void init_power_pins(void);

/**
 * @brief      Initializes the grideye
 */
void grideye_init(void)
{
	/* Initialize config structure and software module. */
	struct i2c_master_config config_i2c_master;
	i2c_master_get_config_defaults(&config_i2c_master);

	/* Change buffer timeout to something longer. */
	config_i2c_master.buffer_timeout = GE_I2C_BUFFER_TIMEOUT;
	config_i2c_master.pinmux_pad0 = GE_SERCOM_PAD0;
	config_i2c_master.pinmux_pad1 = GE_SERCOM_PAD1;
	
	/* Initialize and enable device with config. */
	i2c_master_init(&i2c_master_instance, GE_I2C_MODULE, &config_i2c_master);
	i2c_master_enable(&i2c_master_instance);
	init_power_pins();
}


/**
 * @brief      Initializes the power pins and turns the device on
 */
static void init_power_pins(void) {
	// Configure a pinmux for output
	struct port_config pinmux_out_cfg;
	port_get_config_defaults(&pinmux_out_cfg);
	pinmux_out_cfg.direction = PORT_PIN_DIR_OUTPUT;
	pinmux_out_cfg.input_pull = PORT_PIN_PULL_NONE;

	port_pin_set_config(GE_PWR_PIN, &pinmux_out_cfg);
	port_pin_set_output_level(GE_PWR_PIN, false);
}

/**
 * @brief      Returns whether or not the grideye is sleeping
 *
 * @return     True if the grideye is sleeping, false otherwise
 */
bool ge_is_sleeping(void) {
	return ge_mode == GE_MODE_SLEEP;
}

/**
 * @brief      Sets the mode of the grideye
 *
 * @param[in]  mode  The mode
 *
 * @return     True if the mode was valid, false otherwise
 */
bool ge_set_mode(uint8_t mode) {
	if (ge_mode == mode) {
		return false;  // Already in this mode
	}
	if (ge_mode != GE_MODE_NORMAL) {
		if (mode != GE_MODE_NORMAL) {
			return false;  // Other modes can only transition to normal state
		}
	}
	write_byte(GE_REG_STATE, &mode, 1);

	if (ge_mode == GE_MODE_SLEEP) {  // This must be a wakeup command
		uint8_t rst_cmd;
		// Wait 50ms
		delay_ms(50);
		// Write inital reset
		rst_cmd = GE_CMD_INITIAL_RESET;
		write_byte(GE_REG_RESET, &rst_cmd, 1);
		// Wait 2ms
		delay_ms(2);
		// Write flag reset
		rst_cmd = GE_CMD_FLAG_RESET;
		write_byte(GE_REG_RESET, &rst_cmd, 1);
		// Wait 2 frames
		delay_ms(200);
	}
	ge_mode = mode;
	return true;
}

/**
 * @brief      Gets ambient temperature in Celsius
 *
 * @return     Ambient temperature from the grideye sensor
 */
double ge_get_ambient_temp(void)
{
	uint8_t lsb, msb;

	lsb = read_byte(GE_REG_THERM_LSB);
	msb = read_byte(GE_REG_THERM_MSB);
	return (((msb << 8) + lsb) * 0.0625);
}

/**
 * @brief      Gets a frame from the GridEye
 *
 * @param      frame_buffer  Buffer to read frames into
 */
void ge_get_frame(uint16_t *frame_buffer)
{
	uint8_t lsb, msb;
	for (int i = 0; i < GE_FRAME_SIZE; i++) {
		lsb = read_byte(GE_REG_PIXEL_BASE + 2*i);
		msb = read_byte(GE_REG_PIXEL_BASE + 2*i + 1);
		frame_buffer[i] = ((msb << 8) + lsb);
		// Convert to Celcius (temp = raw * 0.25)
		// frame_buffer[i] = (((msb << 8) + lsb) * 0.25);
	}
}

/**
 * @brief      Reads a byte via I2C.
 *
 * @param[in]  addr  Address to read from
 *
 * @return     Value read from the address
 */
static uint8_t read_byte(uint8_t addr) {
	uint16_t timeout = 0;
	struct i2c_master_packet packet = {
		.address = GE_SLAVE_ADDRESS,
		.data_length = 1,
		.ten_bit_address = false,
		.high_speed = false,
		.hs_master_code = 0x0,
	};
	
	/* Write addr to read */
	ge_write_buffer[0] = addr;
	packet.data = ge_write_buffer;
	while (i2c_master_write_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			break;
		}
	}
	
	/* Read value */
	timeout = 0;
	packet.data = ge_read_buffer;
	while (i2c_master_read_packet_wait(&i2c_master_instance, &packet) !=
	STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			break;
		}
	}
	
	return ge_read_buffer[0];
}

/**
 * @brief      Writes bytes to the given address
 *
 * @param[in]  addr    Address to write to
 * @param      data    Data to write
 * @param[in]  length  Length of data
 */
static void write_byte(uint8_t addr, uint8_t *data, uint8_t length) {
	uint16_t timeout = 0;
	struct i2c_master_packet packet = {
		.address = GE_SLAVE_ADDRESS,
		.data_length = length + 1,
		.ten_bit_address = false,
		.high_speed = false,
		.hs_master_code = 0x0,
	};

	// Populate write buffer [addr, <data>...]
	ge_write_buffer[0] = addr;
	for (uint8_t i = 0; i < length; i++) {
		ge_write_buffer[i+1] = data[i];
	}
	packet.data = ge_write_buffer;
	while (i2c_master_write_packet_wait(&i2c_master_instance, &packet) != STATUS_OK) {
		/* Increment timeout counter and check if timed out. */
		if (timeout++ == TIMEOUT) {
			break;
		}
	}
}