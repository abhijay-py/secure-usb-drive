/*
 * flash.c
 *
 *  Created on: Feb 14, 2025
 *      Author: Jay62
 *
 *
 */

#include "flash.h"

//Instruction Opcodes
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
const uint8_t FLASH_BAD_BLOCK_MANAGEMENT = 0xA1;
const uint8_t FLASH_BAD_BLOCK_LUT = 0xA5;
const uint8_t FLASH_ECC_PAGE_ADDRESS = 0xA9;

//Status Register Addresses
const uint8_t STATUS_REGISTER_ONE = 0xA0;
const uint8_t STATUS_REGISTER_TWO = 0xB0;
const uint8_t STATUS_REGISTER_THREE = 0xC0;

//SPI Holder
SPI_HandleTypeDef *hspiflash;
int *flash_chip_num;

//USB Data Wrapper Functions

//Reset flash ICs and pass in spi.
void flash_init(SPI_HandleTypeDef *hspi1, int* active_flash_chip) {
	hspiflash = hspi1;
	flash_chip_num = active_flash_chip;
	reset_ic(1);
	reset_ic(2);
	reset_ic(3);
	reset_ic(4);
}

//Fill up buf from USB with blk_len blocks from blk_addr.
int flash_read(uint8_t* buf, uint32_t blk_addr, uint16_t blk_len) {

}

//Checks if pointers have been passed in to flash_init()
Flash_Status check_pointers() {
	if (hspiflash == NULL || flash_chip_num == NULL) {
		return FLASH_POINTERS_UNINITIALIZED;
	}
	return FLASH_OK;
}



//Direct Data Access Functions
//ALL Functions assume pins are reset after use.

//Reset IC
Flash_Status reset_ic(int reset_num) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}
	uint8_t tx_buffer[1] = {FLASH_RESET};

	Flash_Status error_one = pin_setup(reset_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 1, 1000);
	pin_setup(reset_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}
	return FLASH_OK;
}

//Transfer page to data buffer
Flash_Status flash_page_read(uint16_t page_address) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}
	uint8_t tx_buffer[4] = {FLASH_PAGE_READ, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;
	
	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Requires flash_page_read to function properly. Also does a continuous read so must have status register set correctly.
//rx_buffer[4:] gets all the data. ASSUME rx_buffer is the right size.
Flash_Status flash_data_read(uint16_t bytes_or_pages, uint8_t* rx_buffer, int count_in_pages) {
	uint16_t size;

	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	if (count_in_pages != 0) {
		if (bytes_or_pages > 31) {
			return FLASH_INVALID_READ_SIZE;
		}
		size = bytes_or_pages * 2048 + 4;
	}
	else {
		if (bytes_or_pages > 63492) {
			return FLASH_INVALID_READ_SIZE;
		}
		size = bytes_or_pages + 4;
	}

	uint8_t tx_buffer[63492] = {0};
	tx_buffer[0] = FLASH_READ_FROM_CACHE;

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, size, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//ASSUME tx_buffer is right size (3 + bytes). Bytes 0-2 will be overwritten. Must disable write protections (block prot and status register writes)
//Column Address (CA) only requires CA[11:0], CA[15:12] are considered as dummy bits.
Flash_Status flash_data_write(uint16_t column_address, uint16_t bytes, uint8_t* tx_buffer, int overwrite_other_data) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	if (bytes > 2048) {
		return FLASH_INVALID_WRITE_SIZE;
	}
	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	if (overwrite_other_data == 1) {
		tx_buffer[0] = FLASH_PROGRAM_LOAD;
	}
	else {
		tx_buffer[0] = FLASH_PROGRAM_LOAD_RANDOM;
	}
	tx_buffer[1] = column_address >> 8;
	tx_buffer[2] = column_address & 0xff;

	pin_setup(*flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, bytes + 3, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	tx_wel_buffer[0] = FLASH_WRITE_DISABLE;

	pin_setup(*flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Commits data write changes.
Flash_Status flash_page_write(uint16_t page_address) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	uint8_t tx_buffer[4] = {FLASH_PROGRAM_EXECUTE, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;

	pin_setup(*flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Erases 128KB (1 block or 64 pages) addressed by PA[15:6]
Flash_Status block_erase(uint16_t page_address) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	uint8_t tx_buffer[4] = {FLASH_BLOCK_ERASE, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;

	pin_setup(*flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Read status
Flash_Status flash_read_status_register(int status_register, uint8_t* data_out) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

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
		case 4:
			return FLASH_INVALID_STATUS_REG;
	}

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 3, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	*data_out = rx_buffer[2];

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Write status
Flash_Status flash_write_status_register(int status_register, uint8_t value) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

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
		case 4:
			return FLASH_INVALID_STATUS_REG;
	}

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 3, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Writes page address of ecc failure in address_out
Flash_Status flash_ecc_failure_address(uint16_t* address_out) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_buffer[4] = {FLASH_ECC_PAGE_ADDRESS, 0x00, 0x00, 0x00};
	uint8_t rx_buffer[4];

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 4, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	*address_out = rx_buffer[3] << 8 | rx_buffer[4];

	return FLASH_OK;
}

//Add to BBM LUT
Flash_Status flash_remap_bad_block(uint16_t bad_block_address, uint16_t new_block_address) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	uint8_t tx_buffer[5] = {FLASH_BAD_BLOCK_MANAGEMENT, 0x00, 0x00, 0x00, 0x00};
	tx_buffer[1] = bad_block_address >> 8;
	tx_buffer[2] = bad_block_address & 0xff;
	tx_buffer[3] = new_block_address >> 8;
	tx_buffer[4] = new_block_address & 0xff;

	pin_setup(*flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 5, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Read BBM LUT, rx_buffer should be 82 bytes.
Flash_Status flash_read_bad_block_LUT(uint8_t* rx_buffer) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_buffer[82] = {0};
	tx_buffer[0] = FLASH_BAD_BLOCK_LUT;

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 82, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Setup External GPIO Pins
Flash_Status pin_setup(int chip_num, int cs, int wp, int hold){
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	if (wp != -1) {
		switch (chip_num) {
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
				return FLASH_INVALID_CHIP_NUM;
		}
	}
	if (hold != -1) {
		switch (chip_num) {
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
				return FLASH_INVALID_CHIP_NUM;
		}
	}
	if (cs != -1) {
		switch (chip_num) {
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
				return FLASH_INVALID_CHIP_NUM;
		}
	}
	return FLASH_OK;
}

//Read manufacturer ID
Flash_Status flash_read_jedec_id(int debug) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_buffer[5] = {FLASH_READ_JEDEC_ID, 0x00, 0x00, 0x00, 0x00};
	uint8_t rx_buffer[5] = {0x00, 0x00, 0x00, 0x00, 0x00};

	Flash_Status error_one = pin_setup(*flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 5, 1000);
	pin_setup(*flash_chip_num, 1, -1, -1);

	if (debug != 0) {
		if (rx_buffer[2] == 0xEF && rx_buffer[3] == 0xAA && rx_buffer[4] == 0x21) {
			Write_Pin(DEBUG_P_EIGHT, 1);
		}
		else {
			Write_Pin(DEBUG_P_EIGHT, 0);
		}
	}
	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}
	return FLASH_OK;
}
