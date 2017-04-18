/*
 * people_counting_utils.c
 *
 * Created: 4/8/2017 10:09:15 PM
 *  Author: Terence Sun (tsun1215)
 */

#include <drivers/people_counting/people_counting_utils.h>
#include <drivers/grideye/grideye.h>

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
// A utility function to swap two elements
void swap (frame_elem_t *a, frame_elem_t *b ) {
	frame_elem_t t = *a;
	*a = *b;
	*b = t;
}

/* This function is same in both iterative and recursive*/
int16_t partition (frame_t arr, int16_t l, int16_t h) {
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

void quick_sort (frame_t arr, int16_t l, int16_t h)
{
    // Create an auxiliary stack
    int16_t stack[ h - l + 1 ];

    // initialize top of stack
    int16_t top = -1;

    // push initial values of l and h to stack
    stack[ ++top ] = l;
    stack[ ++top ] = h;

    // Keep popping from stack while is not empty
    while ( top >= 0 )
    {
        // Pop h and l
        h = stack[ top-- ];
        l = stack[ top-- ];

        // Set pivot element at its correct position
        // in sorted array
        int16_t p = partition( arr, l, h );

        // If there are elements on left side of pivot,
        // then push left side to stack
        if ( p-1 > l )
        {
            stack[ ++top ] = l;
            stack[ ++top ] = p - 1;
        }

        // If there are elements on right side of pivot,
        // then push right side to stack
        if ( p+1 < h )
        {
            stack[ ++top ] = p + 1;
            stack[ ++top ] = h;
        }
    }
}


uint16_t median_at_index(frame_t *frames, uint16_t num_frames, uint16_t index) {
	frame_elem_t temp_arr[num_frames];
	frame_elem_t temp;
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

frame_t compute_median_frame(frame_t frame_out, frame_t *frames, uint16_t num_frames) {
	for (int i = 0; i < GE_FRAME_SIZE; i++) {
		frame_out[i] = median_at_index(frames, num_frames, i);
	}
	return frame_out;
}


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
