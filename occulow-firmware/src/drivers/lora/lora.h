/*
 * lora.h
 *
 * Created: 3/24/2017 12:08:35 PM
 *  Author: mazacar8
 */

#ifndef LORA_H_
#define LORA_H_
#include <asf.h>

#define LORA_SERCOM_PINMUX_PAD0 PINMUX_UNUSED
#define LORA_SERCOM_PINMUX_PAD1 PINMUX_UNUSED
#define LORA_SERCOM_PINMUX_PAD2 PINMUX_PA18C_SERCOM1_PAD2
#define LORA_SERCOM_PINMUX_PAD3 PINMUX_PA19C_SERCOM1_PAD3
#define LORA_USART_MUX_SETTING USART_RX_3_TX_2_XCK_3
#define LORA_USART_BAUD 57600
#define LORA_POWER_PIN PIN_PA14

#define LORA_RESET_PIN PIN_PA15

#define MAX_JOIN_ATTEMPTS 5
#define MAX_STATUS_CHECKS 5
#define MAX_READ_ATTEMPTS 100
#define LORA_RX_BUFFER_SIZE 100
#define LORA_TX_BUFFER_SIZE 100
#define LORA_REJOIN_DELAY 2000

typedef enum {OK, ACCEPTED, DENIED, UNKNOWN} lora_status_t;
typedef uint8_t *lora_cmd_t;

void lora_init(void);
void lora_join_otaa(void);
void lora_join_abp(void);
void lora_reset(void);
void lora_sleep(void);
void lora_wake(void);
lora_status_t lora_send_cmd(lora_cmd_t cmd, uint16_t len);
void lora_send_data(uint8_t *string, uint16_t len);
void lora_send_count(uint16_t ingress, uint16_t egress);

#endif /* LORA_H_ */