/*
 * flash.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Jay62
 */
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



void reset_ic(int flash_chip_num) {
	switch (flash_chip_num) {
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		default:
			break;
	}
}
