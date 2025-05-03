/*
 * keypad.h
 *
 *  Created on: Feb 12, 2025
 *      Author: sos
 */

#ifndef INC_KEYPAD_H_
#define INC_KEYPAD_H_

#include "stdint.h"

#define NUM_ROWS 4
#define NUM_COLS 3

char key_to_char(int row, int col);
int is_key_pressed(int row, int col, int debouncing);

/* EXAMPLE CODE
if (keypad_matrix[3][0] >= 10) { // Debounce for 40ms (10*4 = 40)
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);
} else {
	HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
}
*/

#endif /* INC_KEYPAD_H_ */
