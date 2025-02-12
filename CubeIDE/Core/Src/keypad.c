/*
 * keypad.c
 *
 *  Created on: Feb 12, 2025
 *      Author: sos
 */

#include "keypad.h"
#include "main.h"

volatile int current_row = 0;

// Callback: timer has reset
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	// Check which version of the time triggered this callback
	if (htim == &htim7) {
		uint32_t column_data = (GPIOB->IDR >> 4) & 0b111;

		if (current_row == 3 && (column_data & 0b001) != GPIO_PIN_RESET) { // (0,0) is pressed
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		}

		if (current_row == 3 && (column_data & 0b100) != GPIO_PIN_RESET) { // (0,0) is pressed
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_SET);
		} else {
			HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, GPIO_PIN_RESET);
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
