/*
 * pir.h
 *
 * Created: 4/7/2017 4:57:50 PM
 *  Author: tsun
 */

#ifndef PIR_H_
#define PIR_H_

#include <asf.h>

// Port that PIR_TRIG is connected to
#define PIR_PORT PIN_PA06
// External interrupt channel of the PIR
#define PIR_EIC_LINE PIN_PA06A_EIC_EXTINT6
// PIR EIC pinmux
#define PIR_EIC_PINMUX PINMUX_PA06A_EIC_EXTINT6
// Port that PIR_VCC_CTRL is connected to
#define PIR_PWR_CTRL PIN_PA07

/**
 * @brief      Initializes the PIR to wake up the device and call the given
 *             wakeup callback function
 *
 * @param[in]  on_wake  Callback function to call on wakeup
 */
void pir_init(extint_callback_t on_wake);
void pir_enable_interrupt(void);
void pir_disable_interrupt(void);

#endif /* PIR_H_ */