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

// Callback: timer has reset
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// Check which version of the time triggered this callback
	if (htim == &htim7) {
		uint32_t column_data = (GPIOB->IDR >> 4) & 0b111;

		// update keypad_matrix (debouncing)
		for (int i = 0; i < NUM_COLS; i++) {
			if ((column_data & (1 << i)) != GPIO_PIN_RESET) { // That column is pressed
				keypad_matrix[current_row][i]++;
			} else {
				keypad_matrix[current_row][i] = 0;
			}
		}

		HAL_GPIO_WritePin(GPIOB, (uint16_t)0x000F, GPIO_PIN_RESET); // Set the output of all the rows to 0
		if (current_row == 3) {
			current_row = 0;
		} else {
			current_row++;
		}
		HAL_GPIO_WritePin(GPIOB, (uint16_t)(1 << current_row), GPIO_PIN_SET); // Set the output of the current row to 1
	}
}
