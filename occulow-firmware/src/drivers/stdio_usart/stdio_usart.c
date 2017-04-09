/*
 * stdio_usart.c
 *
 * Created: 4/8/2017 9:24:58 PM
 *  Author: tsun
 */ 

#include <drivers/stdio_usart/stdio_usart.h>

struct usart_module stdio_usart_module;

void stdio_init() {
	struct usart_config stdio_usart_config;
	usart_get_config_defaults(&stdio_usart_config);
	
	stdio_usart_config.mux_setting = STDIO_SERCOM_MUX_SETTING;
	stdio_usart_config.pinmux_pad0 = STDIO_SERCOM_PINMUX_PAD0;
	stdio_usart_config.pinmux_pad1 = STDIO_SERCOM_PINMUX_PAD1;
	stdio_usart_config.pinmux_pad2 = STDIO_SERCOM_PINMUX_PAD2;
	stdio_usart_config.pinmux_pad3 = STDIO_SERCOM_PINMUX_PAD3;
	stdio_usart_config.baudrate = STDIO_BAUD_RATE;
	stdio_serial_init(&stdio_usart_module, STDIO_SERCOM_MODULE, &stdio_usart_config);
	usart_enable(&stdio_usart_module);
}