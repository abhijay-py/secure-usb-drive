/*
 * flash.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Jay62
 *
 *
 */

#include "flash.h"

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
const uint8_t FLASH_READ_JEDEC_ID = 0x9F;

const uint8_t STATUS_REGISTER_ONE = 0xA0;
const uint8_t STATUS_REGISTER_TWO = 0xB0;
const uint8_t STATUS_REGISTER_THREE = 0xC0;

void pin_setup(int flash_chip_num, int cs, int wp, int hold){
	if (wp != -1) {
		switch (flash_chip_num) {
			case 1:
				Write_Pin(FLASH_P_WP_ONE, wp);
				break;
			case 2:
				Write_Pin(FLASH_P_WP_TWO, wp);
				break;
			case 3:
				Write_Pin(FLASH_P_WP_THREE, wp);
				break;
			case 4:
				Write_Pin(FLASH_P_WP_FOUR, wp);
				break;
			default:
				return;
		}
	}
	if (hold != -1) {
		switch (flash_chip_num) {
			case 1:
				Write_Pin(FLASH_P_HOLD_ONE, hold);
				break;
			case 2:
				Write_Pin(FLASH_P_HOLD_TWO, hold);
				break;
			case 3:
				Write_Pin(FLASH_P_HOLD_THREE, hold);
				break;
			case 4:
				Write_Pin(FLASH_P_HOLD_FOUR, hold);
				break;
			default:
				return;
		}
	}
	if (cs != -1) {
		switch (flash_chip_num) {
			case 1:
				Write_Pin(FLASH_P_CS_ONE, cs);
				break;
			case 2:
				Write_Pin(FLASH_P_CS_TWO, cs);
				break;
			case 3:
				Write_Pin(FLASH_P_CS_THREE, cs);
				break;
			case 4:
				Write_Pin(FLASH_P_CS_FOUR, cs);
				break;
			default:
				return;
		}
	}
}

void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num) {
	HAL_Delay(1000);

	pin_setup(flash_chip_num, 0, -1, -1);
	HAL_SPI_Transmit(hspi1, &FLASH_RESET, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);
}

void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num, int debug) {
	pin_setup(flash_chip_num, 0, -1, -1);

	uint8_t tx_buffer[5] = {FLASH_READ_JEDEC_ID, 0x00, 0x00, 0x00, 0x00};
	uint8_t rx_buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	HAL_SPI_TransmitReceive(hspi1, tx_buffer, rx_buffer, 5, 1000);

	pin_setup(flash_chip_num, 1, -1, -1);

	if (debug != 0) {
		if (rx_buffer[2] == 0xEF && rx_buffer[3] == 0xAA && rx_buffer[4] == 0x21) {
			Write_Pin(DEBUG_P_EIGHT, 1);
		}
		else {
			Write_Pin(DEBUG_P_EIGHT, 0);
		}
	}
}

uint8_t flash_read_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register) {
	uint8_t tx_buffer[3] = {FLASH_READ_STATUS_REGISTER, 0x00, 0x00};
	uint8_t rx_buffer[3] = {0x00, 0x00, 0x00};

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

	pin_setup(flash_chip_num, 0, -1, -1);
	HAL_SPI_TransmitReceive(hspi1, tx_buffer, rx_buffer, 3, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	return rx_buffer[3];
}

void flash_write_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register, uint8_t value) {0
	uint8_t tx_buffer[3] = {FLASH_WRITE_STATUS_REGISTER, 0x00, value};

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

	pin_setup(flash_chip_num, 0, -1, -1);
	HAL_SPI_Transmit(hspi1, tx_buffer, 3, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

}

void flash_page_transfer(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t page_address) {
	uint8_t tx_buffer[4] = {FLASH_PAGE_READ, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;
	
	pin_setup(flash_chip_num, 0, -1, -1);
	HAL_SPI_Transmit(hspi1, tx_buffer, 4, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);
}

//MAX Transmit size 65536 bytes both ways.
