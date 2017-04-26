/*
 * people_counting_utils.c
 *
 * Created: 4/8/2017 10:09:15 PM
 *  Author: Terence Sun (tsun1215)
 */

#include <drivers/people_counting/people_counting_utils.h>
#include <drivers/grideye/grideye.h>

/**
 * @brief      Enqueues new_frame into frame_queue dropping the oldest frame
 *
 * @param      frame_queue  Queue to enqueue frame into
 * @param[in]  new_frame    New frame
 * @param[in]  queue_size   Length of queue
 */
void enqueue_frame(frame_t *frame_queue, frame_t new_frame, uint16_t queue_size) {
	// Copy new frame data into frame_queue[0]
	for (int i = 0; i < GE_FRAME_SIZE; i++) {
		frame_queue[0][i] = new_frame[i];
	}

	// Rotate all pointers by 1 (placing newest frame at bottom)
	frame_t newest_frame = frame_queue[0];
	for (int i = 0; i < queue_size - 1; i++) {
		frame_queue[i] = frame_queue[i+1];
	}
	frame_queue[queue_size-1] = newest_frame;
}

/**
 * @brief      Swaps two frame_elements
 *
 * @param      a     { parameter_description }
 * @param      b     { parameter_description }
 */
void swap (frame_elem_t *a, frame_elem_t *b) {
	frame_elem_t t = *a;
	*a = *b;
	*b = t;
}

/**
 * @brief      Partitions a frame for quicksort
 *
 * @param[in]  arr   List to partition
 * @param[in]  l     Element to start partitioning from
 * @param[in]  h     Element to stop partitioning at
 *
 * @return     Index of the partition
 */
int16_t partition(frame_t arr, int16_t l, int16_t h) {
	frame_elem_t x = arr[h];
	int16_t i = (l - 1);

	for (int16_t j = l; j <= h - 1; j++) {
		if (arr[j] <= x) {
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[h]);
	return (i + 1);
}

/**
 * @brief      Performs quicksort on a slice of an array from l to h
 *
 * @param[in]  arr   Array to sort
 * @param[in]  l     Index to start sorting from
 * @param[in]  h     Index to stop sorting from
 */
void quick_sort(frame_t arr, int16_t l, int16_t h) {
    int16_t stack[h - l + 1];
    int16_t top = -1;

    stack[++top] = l;
    stack[++top] = h;

    while (top >= 0) {
        h = stack[top--];
        l = stack[top--];

        int16_t p = partition(arr, l, h);

        if (p-1 > l) {
            stack[++top] = l;
            stack[++top] = p - 1;
        }
        if (p+1 < h) {
            stack[++top] = p + 1;
            stack[++top] = h;
        }
    }
}


/**
 * @brief      Gets the median value across all frames at the given index
 *
 * @param      frames      List of frames
 * @param[in]  num_frames  Number of frames
 * @param[in]  index       Index to find the median value for
 *
 * @return     The median value at the given index
 */
uint16_t median_at_index(frame_t *frames, uint16_t num_frames, uint16_t index) {
	frame_elem_t temp_arr[num_frames];
	// Copy elems into temp arr
	for (int i = 0; i < num_frames; i++) {
		temp_arr[i] = frames[i][index];
	}

	// Sort arr
	quick_sort(temp_arr, 0, num_frames-1);

	// Return median
	if (num_frames % 2 == 0) {
		return ((temp_arr[num_frames/2] + temp_arr[num_frames/2 - 1])/2);
	} else {
		return temp_arr[num_frames/2];
	}
}

/**
 * @brief      Calculates the median frame
 *
 * @param[in]  frame_out   Output buffer to put median frame into
 * @param      frames      List of frames
 * @param[in]  num_frames  Number of frames
 *
 * @return     Median frame
 */
frame_t compute_median_frame(frame_t frame_out, frame_t *frames, uint16_t num_frames) {
	uint16_t index;
	// Find median for each element in the frame
	for (uint16_t col = 0; col < GRID_SIZE; col++) {
		for (int16_t row = 0; row < GRID_SIZE; row++) {
			index = GET_FRAME_INDEX(row, col);
			frame_out[index] = median_at_index(frames, num_frames, index);
		}
	}
	
	return frame_out;
}


/**
 * @brief      Returns if a given (row, col) in frame is a local maximum
 *
 * @param[in]  frame  Frame
 * @param[in]  row    Row in frame
 * @param[in]  col    Col in frame
 *
 * @return     True if local maximum, False otherwise.
 */
bool is_local_max(frame_t frame, uint16_t row, uint16_t col) {
	frame_elem_t current_max = frame[GET_FRAME_INDEX(row, col)];

	// Greater than (row+1, col), (row-1, col)
	if ((row < 7 && current_max < frame[GET_FRAME_INDEX(row+1, col)])
		|| (row > 0 && current_max < frame[GET_FRAME_INDEX(row-1, col)])) {
		return false;
	}
	// Greater than (row, col+1), (row, col-1)
	if ((col < 7 && current_max < frame[GET_FRAME_INDEX(row, col+1)])
		|| (col > 0 && current_max < frame[GET_FRAME_INDEX(row, col-1)])) {
		return false;
	}
	// Greater than (row+1, col+1), (row-1, col-1)
	if ((row < 7 && col < 7 && current_max < frame[GET_FRAME_INDEX(row+1, col+1)])
		|| (row > 0 && col > 0 && current_max < frame[GET_FRAME_INDEX(row-1, col-1)])) {
		return false;
	}
	// Greater than (row+1, col-1), (row-1, col+1)
	if ((row < 7 && col > 0 && current_max < frame[GET_FRAME_INDEX(row+1, col-1)])
		|| (row > 0 && col < 7 && current_max < frame[GET_FRAME_INDEX(row-1, col+1)])) {
		return false;
	}
	return true;
}

/**
 * @brief      Gets the index of the max in a column of frame
 *
 * @param[in]  frame  Frame to find a max in
 * @param[in]  col    Column of frame to find max in
 *
 * @return     The index of the maximum value in the given column of frame
 */
uint16_t get_max_index_in_col(frame_t frame, uint16_t col) {
	// Initialize max to the 0th element in the column
	frame_elem_t max_elem = frame[GET_FRAME_INDEX(0, col)];
	uint16_t max_elem_index = 0;

	for (int i = 0; i < GRID_SIZE; i++) {
		frame_elem_t elem = frame[GET_FRAME_INDEX(i, col)];
		if (elem > max_elem) {
			// New max found
			max_elem = elem;
			max_elem_index = i;
		}
	}

	return max_elem_index;
}
