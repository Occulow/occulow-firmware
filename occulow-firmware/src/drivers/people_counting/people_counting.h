/*
 * people_counting.h
 *
 * Created: 3/24/2017 8:59 PM
 * Author: Udaya Malik
 */

#ifndef PEOPLE_COUNTING_H_
#define PEOPLE_COUNTING_H_

#include <asf.h>

#define GRID_SIZE 8
#define NUM_MEDIAN_FRAMES 7
#define MEDIAN_FRAME_CHUNK_SIZE (NUM_MEDIAN_FRAMES * GE_FRAME_SIZE)
#define NUM_RAW_FRAMES 10
#define RAW_FRAME_CHUNK_SIZE (NUM_RAW_FRAMES * GE_FRAME_SIZE)

// Defines for triggers
#define TRIGGER_COLUNM_2 2
#define CHECK_OFFSET_PLUS_1 1
#define TRIGGER_COLUNM_5 5
#define CHECK_OFFSET_MINUS_1 -1
#define TRIGGER_INDEX (NUM_MEDIAN_FRAMES/2)

// Thresholds for detecting motion
#define TRIGGER_THRESHOLD 10  /// Threshold to detect as motion
#define MAX_THRESHOLD 3  /// Threshold for heat signature difference in frames

// Sizes for frames and their elements
typedef uint16_t *frame_t;
typedef uint16_t frame_elem_t;

void pc_init(void);
void pc_new_frame(uint16_t *new_frame);
double pc_get_in_count(void);
double pc_get_out_count(void);

#endif /* PEOPLE_COUNTING_H_ */