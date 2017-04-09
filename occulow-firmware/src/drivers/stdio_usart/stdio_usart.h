/*
 * stdio_usart.h
 *
 * Created: 4/8/2017 9:24:47 PM
 *  Author: tsun
 */ 


#ifndef STDIO_USART_H_
#define STDIO_USART_H_

#include <asf.h>

#define STDIO_SERCOM_PINMUX_PAD0 PINMUX_UNUSED
#define STDIO_SERCOM_PINMUX_PAD1 PINMUX_UNUSED
#define STDIO_SERCOM_PINMUX_PAD2 PINMUX_PA24C_SERCOM3_PAD2
#define STDIO_SERCOM_PINMUX_PAD3 PINMUX_PA25C_SERCOM3_PAD3
#define STDIO_SERCOM_MODULE SERCOM3
#define STDIO_SERCOM_MUX_SETTING USART_RX_3_TX_2_XCK_3

void stdio_init(void);

#endif /* STDIO_USART_H_ */