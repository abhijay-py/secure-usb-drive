/*
 * keypad.c
 *
 *  Created on: Feb 12, 2025
 *      Author: sos
 */

#include "keypad.h"
#include "main.h"

volatile uint32_t keypad_matrix[NUM_ROWS][NUM_COLS] = {0};

static volatile int current_row = 0;

// Input row (0-3) and return the corresponding IC_Pin
IC_Pin row_to_pin(int row) {
	switch (row) {
		case 0: return KEY_P_R_ONE;
		case 1: return KEY_P_R_TWO;
		case 2: return KEY_P_R_THREE;
		case 3: return KEY_P_R_FOUR;
	}
	// If code reaches here user did not enter valid row
	return KEY_P_R_ONE;
}

// Input column (0-2) and return the corresponding IC_Pin
IC_Pin col_to_pin(int col) {
	switch (col) {
		case 0: return KEY_P_C_ONE;
		case 1: return KEY_P_C_TWO;
		case 2: return KEY_P_C_THREE;
	}
	// If code reaches here user did not enter valid column
	return KEY_P_C_ONE;
}

// Callback: timer has reset
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// Check which version of the time triggered this callback
	if (htim == &htim7) {
		// update keypad_matrix (debouncing)
		for (int i = 0; i < NUM_COLS; i++) {
			int col_val = Read_Pin(col_to_pin(i));
			if (col_val != 0) { // That column is pressed
				keypad_matrix[current_row][i]++;
			} else {
				keypad_matrix[current_row][i] = 0;
			}
		}

		Write_Pin(row_to_pin(current_row), 0); // Set the output of the previous row to 0
		if (current_row == 3) {
			current_row = 0;
		} else {
			current_row++;
		}
		Write_Pin(row_to_pin(current_row), 1); // Set the output of the current row to 1

	}
}

