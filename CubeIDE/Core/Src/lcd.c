/*
 * lcd.c
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */
#include "main.h"
#include "lcd.h"

const uint8_t FN_SET_CMD = 0x38;
const uint8_t LCD_ON_CMD = 0x41;
const uint8_t LCD_OFF_CMD = 0x42;
const uint8_t LCD_CLR_CMD = 0x51;
const uint8_t LCD_ENTRY_CMD = 0x06;
const uint8_t LCD_HOME_CMD = 0x46;
const uint8_t LCD_CURS_LOC_CMD = 0x45;
const uint8_t LCD_PREFIX = 0xFE;

void lcd_display_first_row(SPI_HandleTypeDef *hspi2, uint8_t *string) {
//	HAL_SPI_Transmit(hspi2, &LCD_ON_CMD, 1, 1000); //Sending in normal mode
//	HAL_Delay(100);
//	send_string(hspi2, string);
}
//
//void lcd_display_second_row(SPI_HandleTypeDef *hspi2, uint8_t *string) {
//	HAL_SPI_Transmit(hspi2, LCD_2ND_ROW_CMD, 1, 1000); //Sending in normal mode
//	HAL_Delay(100);
//	send_string(hspi2, string);
//}

void lcd_cursor_on(SPI_HandleTypeDef *hspi2) {
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
//	HAL_SPI_Transmit(hspi2, &LCD_CURSOR_ON_BLINK, 1, 1000); //Sending in normal mode
////	HAL_Delay(100);
//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

// note that the hex location ranges from column 0x00 to 0x0F for row 1
// note that the hex location ranges from column 0x40 to 0x4F for row 2
void lcd_cursor_location(SPI_HandleTypeDef *hspi2, const uint8_t hex_location) {
	HAL_Delay(1000);
	if (hex_location == HOME) {
		// call home command
	}
	else {
		const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_CURS_LOC_CMD, hex_location};
		send_bytes(hspi2, tx_buffer, sizeof(tx_buffer));
	}
}

void lcd_welcome(SPI_HandleTypeDef *hspi2) {
	const uint8_t tx_buffer[] = {'w','e','l','c','o','m','e',' ',
								'2',' ',
								'e','c','e','4','7','7'};
	send_bytes(hspi2, tx_buffer, sizeof(tx_buffer));
}

void lcd_clear(SPI_HandleTypeDef *hspi2) {
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_CLR_CMD};
	send_bytes(hspi2, tx_buffer, sizeof(tx_buffer));
}

void lcd_on(SPI_HandleTypeDef *hspi2) {
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_ON_CMD};
	send_bytes(hspi2, tx_buffer, sizeof(tx_buffer));
}

void lcd_off(SPI_HandleTypeDef *hspi2) {
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_OFF_CMD};
	send_bytes(hspi2, tx_buffer, sizeof(tx_buffer));
}

void send_bytes(SPI_HandleTypeDef *hspi2, const uint8_t *tx_buffer, size_t size) {
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	for (int i = 0; i < size; i++) {
		HAL_SPI_Transmit(hspi2, tx_buffer + i, 1, 1000);
		while(hspi2 -> State != HAL_SPI_STATE_READY);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void periodic_send(SPI_HandleTypeDef *hspi2) {

	uint8_t TX_Buffer [] = "A" ;

	HAL_SPI_Transmit(hspi2, TX_Buffer, 1, 1000); //Sending in normal mode
	HAL_Delay(100);
}






