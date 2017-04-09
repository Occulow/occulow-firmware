/*
 * people_counting_utils.h
 *
 * Created: 4/8/2017 10:09:27 PM
 *  Author: Terence Sun (tsun1215)
 */ 


#ifndef PEOPLE_COUNTING_UTILS_H_
#define PEOPLE_COUNTING_UTILS_H_

#include <asf.h>
#include <drivers/people_counting/people_counting.h>

/**
 * @brief      Gets the index into a frame of size GRID_SIZE * GRID_SIZE
 *
 * @param      ROW   Row index
 * @param      COL   Column index
 *
 * @return     Converted index from 2D to 1D
 */
#define GET_FRAME_INDEX(ROW, COL) ((ROW) * GRID_SIZE + (COL))

void enqueue_frame(frame_t *frame_queue, frame_t new_frame, uint16_t queue_size);
bool is_local_max(frame_t frame, uint16_t row, uint16_t col);
uint16_t get_max_index_in_col(frame_t frame, uint16_t col);

#endif /* PEOPLE_COUNTING_UTILS_H_ */