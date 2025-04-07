/*
 * flash.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Jay62
 *
 *
 */

#include "flash.h"

//const uint8_t FLASH_RESET = 0b11111111;
//const uint8_t FLASH_WRITE_ENABLE = 0b00000110;
//const uint8_t FLASH_WRITE_DISABLE = 0b00000100;
//const uint8_t FLASH_PAGE_READ = 0b00011101;
//const uint8_t FLASH_READ_FROM_CACHE = 0b00000011;
//const uint8_t FLASH_PROGRAM_EXECUTE = 0b00010000;
//const uint8_t FLASH_PROGRAM_LOAD = 0b00000010;
//const uint8_t FLASH_PROGRAM_LOAD_RANDOM = 0b10000100;
//const uint8_t FLASH_BLOCK_ERASE = 0b11011000;
//const uint8_t FLASH_GET_FEATURE = 0b00001111;
//const uint8_t FLASH_SET_FEATURE = 0b00011111;
//const uint8_t FLASH_READ_JEDEC_ID = 0b10011111;

const IC_Pin FLASH_P_HOLD = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_6};
const IC_Pin FLASH_P_CHIP_SELECT = (IC_Pin){.pin_letter = GPIOA, .pin_num = GPIO_PIN_0};
const IC_Pin FLASH_P_WRITE_PROTECT = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_2};

const uint8_t FLASH_RESET = 0xFF;
const uint8_t FLASH_WRITE_ENABLE = 0x06;
const uint8_t FLASH_WRITE_DISABLE = 0x04;
const uint8_t FLASH_PAGE_READ = 0x13;
const uint8_t FLASH_READ_FROM_CACHE = 0x03;
const uint8_t FLASH_PROGRAM_EXECUTE = 0x10;
const uint8_t FLASH_PROGRAM_LOAD = 0x02;
const uint8_t FLASH_PROGRAM_LOAD_RANDOM = 0x84;
const uint8_t FLASH_BLOCK_ERASE = 0xD8;
const uint8_t FLASH_READ_STATUS_REGISTER = 0x0F;
const uint8_t FLASH_WRITE_STATUS_REGISTER = 0x1F;
const uint8_t FLASH_READ_JEDEC_ID[5] = {0x9F, 0x00, 0x00, 0x00, 0x00};

const uint8_t STATUS_REGISTER_ONE = 0xA0;
const uint8_t STATUS_REGISTER_TWO = 0xB0;
const uint8_t STATUS_REGISTER_THREE = 0xC0;

void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num) {
	HAL_Delay(1000);

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
			HAL_SPI_Transmit(hspi1, &FLASH_RESET, 1, 1000);
			while(hspi1 -> State != HAL_SPI_STATE_READY);
			HAL_Delay(1000);
			HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
}

void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num) {
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_SET);

	uint8_t buffer[5];

	HAL_SPI_TransmitReceive(hspi1, FLASH_READ_JEDEC_ID, buffer, 5, 1000);
	if (buffer[3] == 0xEF) {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);
	}
	else {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	}
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, GPIO_PIN_RESET);
}

uint8_t flash_read_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register) {
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);

	uint8_t tx_buffer[3] = {FLASH_READ_STATUS_REGISTER, 0x00, 0x00};
	uint8_t rx_buffer[3];

	switch (status_register){
		case 1:
			tx_buffer[1] = STATUS_REGISTER_ONE;
			break;
		case 2:
			tx_buffer[1] = STATUS_REGISTER_TWO;
			break;
		case 3:
			tx_buffer[1] = STATUS_REGISTER_THREE;
			break;
	}

	HAL_SPI_TransmitReceive(hspi1, tx_buffer, rx_buffer, 3, 1000);

	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

	return rx_buffer[3];
}

void flash_write_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register) {
	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_SET);

	uint8_t tx_buffer[3] = {FLASH_WRITE_STATUS_REGISTER, 0x00, 0b11000};
	uint8_t wel = 0x06;

	switch (status_register){
		case 1:
			tx_buffer[1] = STATUS_REGISTER_ONE;
			break;
		case 2:
			tx_buffer[1] = STATUS_REGISTER_TWO;
			break;
		case 3:
			tx_buffer[1] = STATUS_REGISTER_THREE;
			break;
	}

	HAL_SPI_Transmit(hspi1, &wel, 1, 1000);
	HAL_SPI_Transmit(hspi1, tx_buffer, 3, 1000);

	HAL_Delay(1000);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

}


