/**
 * main.c
 *
 */

#include <asf.h>
#include <drivers/stdio_usart/stdio_usart.h>
#include <drivers/pir/pir.h>
#include <drivers/lora/lora.h>
#include <drivers/grideye/grideye.h>
#include <drivers/people_counting/people_counting.h>

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
	lora_init();
	grideye_init();
	pc_init();
	// pir_init(pir_on_wake);  // Init PIR
	
	lora_join_otaa();

	double old_in_count = 0;
	double old_out_count = 0;
	double in_count = 0;
	double out_count = 0;
	
	while(1) {
		if (!ge_is_sleeping()) {
			ge_get_frame(grideye_frame);
			pc_new_frame(grideye_frame);
			in_count = pc_get_in_count();
			out_count = pc_get_out_count();
			
			if (in_count > old_in_count || out_count > old_out_count) {
				lora_send_count((uint16_t) (in_count - old_in_count), 
					(uint16_t) (out_count - old_out_count));
				printf("In: %d, Out: %d\r\n", (int) in_count, (int) out_count);		
				old_in_count = in_count;
				old_out_count = out_count;
			}
		}
	}
}
