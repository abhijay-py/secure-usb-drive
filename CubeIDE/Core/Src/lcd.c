/*
 * lcd.c
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */
#include "main.h"
#include "lcd.h"
#include "stm32f4xx_hal_spi.h"

const uint8_t FN_SET_CMD = 0x38;
const uint8_t LCD_ON_CMD = 0x41; // 0x0c for 362 LCD
const uint8_t LCD_OFF_CMD = 0x42; // 0x08 for 362 LCD
const uint8_t LCD_CLR_CMD = 0x51; // 0x01 for 362 LCD
const uint8_t LCD_ENTRY_CMD = 0x06;
const uint8_t LCD_HOME_CMD = 0x46; // 0x02 for 362 LCD
const uint8_t LCD_2ND_ROW_CMD = 0xc0;
const uint8_t LCD_DATA_PAIR_CMD = 0x02; // this must be the 3rd set of 4 bits
const uint8_t LCD_PREFIX = 0xFE;
const uint8_t LCD_CHAR_CMD = 0x41;

const uint8_t LCD_CURSOR_ON_BLINK = 0x0F;

void lcd_display_first_row(SPI_HandleTypeDef *hspi2, uint8_t *string) {
	HAL_SPI_Transmit(hspi2, &LCD_ON_CMD, 1, 1000); //Sending in normal mode
	HAL_Delay(100);
	send_string(hspi2, string);
}
//
//void lcd_display_second_row(SPI_HandleTypeDef *hspi2, uint8_t *string) {
//	HAL_SPI_Transmit(hspi2, LCD_2ND_ROW_CMD, 1, 1000); //Sending in normal mode
//	HAL_Delay(100);
//	send_string(hspi2, string);
//}

void lcd_cursor_on(SPI_HandleTypeDef *hspi2) {
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, &LCD_CURSOR_ON_BLINK, 1, 1000); //Sending in normal mode
//	HAL_Delay(100);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void lcd_welcome(SPI_HandleTypeDef *hspi2) {
	HAL_Delay(1000);
	const uint8_t tx_buffer[] = {'w','e','l','c','o','m','e',' ',
								'2',' ',
								'e','c','e','4','7','7'};
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	for (int i = 0; i < sizeof(tx_buffer); i++) {
		HAL_SPI_Transmit(hspi2, tx_buffer + i, 1, 1000);
		while(hspi2 -> State != HAL_SPI_STATE_READY);
		HAL_Delay(100);
	}
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void lcd_clear(SPI_HandleTypeDef *hspi2) {
	HAL_Delay(1000);
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_CLR_CMD};

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);

//	const uint8_t tx_buffer2[] = {LCD_CLR_CMD};

//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer + 1, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void lcd_on(SPI_HandleTypeDef *hspi2) {
	HAL_Delay(1000);
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_ON_CMD};

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);

//	const uint8_t tx_buffer2[] = {LCD_CLR_CMD};

//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer + 1, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void lcd_off(SPI_HandleTypeDef *hspi2) {
	HAL_Delay(1000);
	const uint8_t tx_buffer[] = {LCD_PREFIX, LCD_OFF_CMD};

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);

//	const uint8_t tx_buffer2[] = {LCD_CLR_CMD};

//	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_SPI_Transmit(hspi2, tx_buffer + 1, 1, 1000);
	while(hspi2 -> State != HAL_SPI_STATE_READY);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3, GPIO_PIN_SET);
}

void send_string(SPI_HandleTypeDef *hspi2, uint8_t *string) {
	uint8_t data[2] = {LCD_DATA_PAIR_CMD, *string};
//	for (int i = 0; string[i] != '\0'; i++) {
		HAL_SPI_Transmit(hspi2, data, 2, 1000); // Data to be sent in 8 bits
		HAL_Delay(100);
//	}
}

void periodic_send(SPI_HandleTypeDef *hspi2) {

	uint8_t TX_Buffer [] = "A" ;

	HAL_SPI_Transmit(hspi2, TX_Buffer, 1, 1000); //Sending in normal mode
	HAL_Delay(100);
}






