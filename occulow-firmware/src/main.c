/**
 * \file
 *
 * \brief Empty user application template
 *
 */

/**
 * \mainpage User Application template doxygen documentation
 *
 * \par Empty user application template
 *
 * Bare minimum empty user application template
 *
 * \par Content
 *
 * -# Include the ASF header files (through asf.h)
 * -# Minimal main function that starts with a call to system_init()
 * -# "Insert application code here" comment
 *
 */

/*
 * Include header files for all drivers that have been imported from
 * Atmel Software Framework (ASF).
 */
/*
 * Support and FAQ: visit <a href="http://www.atmel.com/design-support/">Atmel Support</a>
 */
#include <asf.h>
#include <drivers/pir/pir.h>
#include <drivers/grideye/grideye.h>

/**
 * @brief      Runs when the PIR detects motion
 */
void pir_on_wake(void);

void pir_on_wake(void) {
	// TODO: Implement what happens when the PIR sends an interrupt
}

int main (void)
{
	uint16_t grideye_frame[NUM_PIXELS];
	system_init();
	grideye_init();
	pir_init(pir_on_wake);  // Init PIR

	while(1) {
		if (!ge_is_sleeping()) {
			ge_get_frame(grideye_frame);
			// TODO: Do something with grideye_frame
		}
	}
}
