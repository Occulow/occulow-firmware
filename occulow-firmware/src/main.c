/**
 * main.c
 *
 */

#include <asf.h>
#include <drivers/pir/pir.h>
#include <drivers/grideye/grideye.h>
#include <drivers/people_counting/people_counting.h>
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
	frame_elem_t grideye_frame[GE_FRAME_SIZE];
	system_init();
	delay_init();
	stdio_init();
	grideye_init();
	pc_init();
	// pir_init(pir_on_wake);  // Init PIR

	double in_count = 0;
	double out_count = 0;
	
	while(1) {
		if (!ge_is_sleeping()) {
			ge_get_frame(grideye_frame);
			pc_new_frame(grideye_frame);
			in_count = pc_get_in_count();
			out_count = pc_get_out_count();
			
			if (in_count > 0.0 || out_count > 0.0) {
				printf("In: %d, Out: %d\r\n", (int) in_count, (int) out_count);
			}
		}
	}
}
