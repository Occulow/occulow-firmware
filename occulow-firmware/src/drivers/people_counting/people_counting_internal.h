/*
 * people_counting_internal.h
 *
 * Created: 4/9/2017 12:03:29 AM
 *  Author: tsun
 */ 


#ifndef PEOPLE_COUNTING_INTERNAL_H_
#define PEOPLE_COUNTING_INTERNAL_H_

/** @brief Counter state */
typedef struct {
	double in_count;  ///< Number of counts going in
	double out_count;  ///< Number of counts going out
	bool count_updated;  ///< Whether or not the count has been updated
} pc_counter_t;

/** @brief People counting configuration */
typedef struct {
	uint16_t trigger_column[2];  ///< Columns to detect movement on
	uint16_t trigger_check_offset[2];  ///< Offsets to check for direction
} pc_config_t;

/** @brief Direction that motion was detected in */
typedef enum {DIR_IN, DIR_OUT, DIR_NONE} direction_t;

static void initialize_counter(pc_counter_t *counter);
static void initialize_frame_stacks(void);
static direction_t determine_direction(uint16_t frame_index, uint16_t trigger_col, uint16_t offset);
static void update_counter(void);

#endif /* PEOPLE_COUNTING_INTERNAL_H_ */