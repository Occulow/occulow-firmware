/*
* people_counting.c
*
* Created: 3/24/2017 10:51:44 PM
*  Author: Udaya Malik
*/

#include <asf.h>
#include <drivers/grideye/grideye.h>
#include <drivers/people_counting/people_counting_utils.h>
#include <drivers/people_counting/people_counting_internal.h>

// Allocate areas to store raw and median frames
static frame_elem_t MEDIAN_FRAME_CHUNK[MEDIAN_FRAME_CHUNK_SIZE];
static frame_elem_t RAW_FRAME_CHUNK[RAW_FRAME_CHUNK_SIZE];

// Allocate lists of pointers into the chunks
static frame_t RAW_FRAMES[NUM_RAW_FRAMES];
static frame_t MEDIAN_FRAMES[NUM_MEDIAN_FRAMES];

static uint16_t raw_frame_count = 0;  ///< Number of frames in the RAW_FRAMES
static bool raw_frames_filled = false;  ///< Whether or not RAW_FRAMES is full
static pc_counter_t pc_counter;  ///< Counter object
static pc_config_t pc_config;  ///< Configuration object


/**
 * @brief      Initializes the people counting driver
 */
void pc_init() {
	initialize_counter(&pc_counter);
	initialize_frame_stacks();
}

/**
 * @brief      Adds a new frame to be processed (for people counting)
 *
 * @param[in]  new_frame  The new frame to be processed
 */
void pc_new_frame(frame_t new_frame) {
	// Check if RAW_FRAMES has been filled
	if (raw_frame_count < NUM_RAW_FRAMES) {
		raw_frame_count++;
	} else {
		raw_frames_filled = true;
	}
	
	// Enqueue new_frame into RAW_FRAMES
	enqueue_frame(RAW_FRAMES, new_frame, NUM_RAW_FRAMES);
	
	// Reset counted
	pc_counter.count_updated = false;
}

/**
 * @brief      Gets the ingress count
 *
 * @return     The ingress count
 */
double pc_get_in_count(void) {
	// Update count if not updated
	if (!pc_counter.count_updated) {
		update_counter();
	}
	return pc_counter.in_count;
}

/**
 * @brief      Gets the egress count
 *
 * @return     The egress count
 */
double pc_get_out_count(void) {
	// Update count if not updated
	if (!pc_counter.count_updated) {
		update_counter();
	}
	return pc_counter.out_count;
}

/**
 * @brief      Initializes a counter object
 */
void initialize_counter(pc_counter_t *counter)
{
	counter->in_count = 0;
	counter->out_count = 0;
	counter->count_updated = false;
}

/**
 * @brief      Initializes the frame stacks
 */
static void initialize_frame_stacks(void) {
	for (int i = 0; i < NUM_RAW_FRAMES; i++) {
		RAW_FRAMES[i] = &RAW_FRAME_CHUNK[i * GE_FRAME_SIZE];
	}
	for (int i = 0; i < NUM_MEDIAN_FRAMES; i++) {
		MEDIAN_FRAMES[i] = &MEDIAN_FRAME_CHUNK[i * GE_FRAME_SIZE];		
	}
}

/**
 * @brief      Detect motion on trigger_col and determine the direction of
 *             movement by checking column at trigger_col + offset
 *
 * @param[in]  frame_index  Index of frame to check for motion
 * @param[in]  trigger_col  Column within the frame to check for motion
 * @param[in]  offset       Offset from trigger_col to determine direction of
 *                          movement
 *
 * @return     Direction of movement if detected, NONE otherwise
 */
static direction_t determine_direction(uint16_t frame_index, 
	uint16_t trigger_col, uint16_t offset) {
	if (TRIGGER_INDEX >= NUM_MEDIAN_FRAMES - 1 || TRIGGER_INDEX < 1){
		//TODO: Handle exception
	}
	if (trigger_col + offset < 0 || trigger_col + offset >= GRID_SIZE) {
		//TODO: Handle exception
	}
	uint16_t check_col = trigger_col + offset;
	frame_t current_frame = MEDIAN_FRAMES[frame_index];
	uint16_t current_max_index = 
		get_max_index_in_col(current_frame, trigger_col);
	uint16_t current_max = 
		current_frame[GET_FRAME_INDEX(current_max_index, trigger_col)];
	
	if (current_max >= TRIGGER_THRESHOLD 
		&& is_local_max(current_frame, current_max_index, trigger_col)) {
		// Check check_col in the past
		// (Change upper bound to look further into the past)
		for (uint16_t i = 1; i < 3; i++) {
			frame_t past_frame = MEDIAN_FRAMES[frame_index - i];
			uint16_t past_max_index = get_max_index_in_col(past_frame, check_col);
			uint16_t past_max = past_frame[GET_FRAME_INDEX(past_max_index, trigger_col)];
			if (abs(past_max - current_max) <= MAX_THRESHOLD 
				&& is_local_max(past_frame, past_max_index, check_col)) {
				if (offset < 0) {
					return DIR_IN;
				} else if (offset > 0) {
					return DIR_OUT;
				}
			}
		}
		
		// Check check_col in the future
		// (Change upper bound to look further into the future)
		for (uint16_t i = 1; i < 3; i++) {
			frame_t future_frame = MEDIAN_FRAMES[frame_index + i];
			uint16_t future_max_index = get_max_index_in_col(future_frame, check_col);
			uint16_t future_max = future_frame[GET_FRAME_INDEX(future_max_index, trigger_col)];
			if (abs(future_max - current_max) <= MAX_THRESHOLD 
				&& is_local_max(future_frame, future_max_index, check_col)) {
				if (offset < 0) {
					return DIR_OUT;
				} else if (offset > 0) {
					return DIR_IN;
				}
			}
		}
	}
	return DIR_NONE;
}

/**
 * @brief      Updates the counter object if it hasn't already been updated
 */
static void update_counter(void) {
	// Exit early if count has already been updated or the buffer isn't full
	if (!raw_frames_filled || pc_counter.count_updated) {
		return;
	}
	
	// Determine the direction for each trigger column
	for (int i = 0; i < 2; i++){
		uint16_t trigger_col = pc_config.trigger_column[i];
		uint16_t offset = pc_config.trigger_check_offset[i];
		int direction = determine_direction(TRIGGER_INDEX, trigger_col, offset);
		
		// Increment count according to number of trigger columns
		// TODO: Is this math too slow?
		if (direction == DIR_IN) {
			pc_counter.in_count = pc_counter.in_count + (1/sizeof(pc_config.trigger_column));
		} else if (direction == DIR_OUT) {
			pc_counter.out_count = pc_counter.out_count + (1/sizeof(pc_config.trigger_column));
		}
	}
	
	// Mark counter as updated for this frame
	pc_counter.count_updated = true;
}
