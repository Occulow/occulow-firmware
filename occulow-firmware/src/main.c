/**
 * main.c
 *
 */

#include <asf.h>
#include <drivers/pir/pir.h>
#include <drivers/grideye/grideye.h>
#include <drivers/stdio_usart/stdio_usart.h>

/**
 * @brief      Runs when the PIR detects motion
 */
void pir_on_wake(void);

void pir_on_wake(void) {
	// TODO: Implement what happens when the PIR sends an interrupt
}

int main (void)
{
	uint16_t grideye_frame[GE_FRAME_SIZE];
	char buffer[512];
	system_init();
	stdio_init();
	grideye_init();
	pir_init(pir_on_wake);  // Init PIR

	while(1) {
		if (!ge_is_sleeping()) {
			ge_get_frame(grideye_frame);
			uint16_t size = 0;
			for (int i = 0; i < GE_FRAME_SIZE; i++) {
				size += sprintf((char *) (buffer + size), "%d,", grideye_frame[i]);
			}
			buffer[size-1] = '\r';  // Replace last comma with \r
			buffer[size] = '\n';
			buffer[size+1] = '\0';
			printf("%s", buffer);
		}
	}
}
