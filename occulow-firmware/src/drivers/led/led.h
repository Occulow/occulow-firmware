/*
 * led.h
 *
 * Created: 4/28/2017 3:50:30 PM
 *  Author: tsun
 */


#ifndef LED_H_
#define LED_H_


#include <asf.h>

#define LED_PIN PIN_PA27

void led_init(void);
void led_set_state(bool state);
bool led_get_state(void);

#endif /* LED_H_ */