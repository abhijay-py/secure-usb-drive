/*
 * flash.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Jay62
 *
 *
 */

#include "main.h"
#include "flash.h"
#include "stm32f4xx_hal_spi.h"

const uint8_t FLASH_RESET = 0b11111111;
const uint8_t FLASH_WRITE_ENABLE = 0b00000110;
const uint8_t FLASH_WRITE_DISABLE = 0b00000100;
const uint8_t FLASH_PAGE_READ = 0b00011101;
const uint8_t FLASH_READ_FROM_CACHE = 0b00000011;
const uint8_t FLASH_PROGRAM_EXECUTE = 0b00010000;
const uint8_t FLASH_PROGRAM_LOAD = 0b00000010;
const uint8_t FLASH_PROGRAM_LOAD_RANDOM = 0b10000100;
const uint8_t FLASH_BLOCK_ERASE = 0b11011000;
const uint8_t FLASH_GET_FEATURE = 0b00001111;
const uint8_t FLASH_SET_FEATURE = 0b00011111;
const uint8_t FLASH_READ_JEDEC_ID = 0b10011111;

void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num) {
	HAL_Delay(1000);
//	switch (flash_chip_num) {
//		case 0:
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
			HAL_SPI_Transmit(hspi1, &FLASH_RESET, 1, 1000);
			while(hspi1 -> State != HAL_SPI_STATE_READY);
			HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//			break;
//		case 1:
//			break;
//		case 2:
//			break;
//		case 3:
//			break;
//		default:
//			break;
//	}
}

void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num) {
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

	uint8_t buffer[8];
//	switch (flash_chip_num) {
//		case 0:
	HAL_SPI_Transmit(hspi1, &FLASH_READ_JEDEC_ID, 1, 1000);
//			while(hspi1 -> State != HAL_SPI_STATE_READY);
	HAL_SPI_TransmitReceive(hspi1, buffer, buffer, 8, 1000); // dummy clock
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
//			break;
//		case 1:
//			break;
//		case 2:
//			break;
//		case 3:
//			break;
//		default:
//			break;
//	}
}
