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

//Flash Parameters
SPI_HandleTypeDef *hspiflash;
int flash_chip_num = 1;
int flash_freeze = 0;
int flash_busy = 0;
uint8_t rx_buffer_full_size[63492] = {0};


//USB Data Wrapper Functions

//Reset flash ICs and pass in spi
void flash_init(SPI_HandleTypeDef *hspi1) {
	hspiflash = hspi1;
	reset_ic(1);
	reset_ic(2);
	reset_ic(3);
	reset_ic(4);
}

//Fill up buf from USB with blk_len blocks from blk_addr.
//Check we don't go over max page addr, blocks in this context means pages as usb will treat pages as blocks
//Max read size of 500 kB or 244 blocks, can also fail if buf not big enough. (not checked for)
int flash_read(uint8_t* buf, uint32_t blk_addr, uint16_t blk_len) {
	uint16_t remaining_block_count = blk_len;
	uint16_t current_address = (blk_addr/4) & 0xFFFF;
	uint16_t remaining_page_count = (blk_len + blk_addr - 1) / 4 - current_address + 1;
	uint16_t buf_index = 0;
	uint16_t page_offset = blk_addr % 4;
	uint16_t rx_buffer_preset = page_offset;
	Flash_Status error;

	//Return immediate of blk_len is 0
	if (blk_len == 0) {
		return 0;
	}

	//Check if we have enough data to transfer from a certain address and pointers are okay
	if ((blk_len + blk_addr) > 262144 || check_pointers() != FLASH_OK) {
		return -1;
	}
	//Returns Busy if flash freeze
	if (flash_freeze == 1) {
		return 1;
	}


	flash_busy = 1;
	//Calculate iterations for loop
	int iterations = remaining_page_count / (31);

	if (iterations * 31 != remaining_block_count) {
		iterations++;
	}

	//Loop until all data is obtained
	for (int i = 0; i < iterations; i++) {
		//Get next page
		error = flash_page_read(current_address);

		if (error != FLASH_OK) {
			return -1;
			flash_busy = 0;
		}
		//Transfer remaining blocks
		if (remaining_page_count < 31) {
			//Read Data in
			error = flash_data_read(remaining_page_count, rx_buffer_full_size, 1);
			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}

			//Transfer data from rx_buffer to buf
			memcpy((void*)(buf + buf_index), (void*)(rx_buffer_full_size + 4 + 512 * rx_buffer_preset), 512*remaining_block_count*sizeof(uint8_t));
		}
		//Transfer max 31 blocks
		else {
			//Read Data in
			error = flash_data_read(31, rx_buffer_full_size, 1);
			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}

			//Transfer data from rx_buffer to buf
			memcpy((void*)(buf + buf_index), (void*)(rx_buffer_full_size + 4 + 512 * rx_buffer_preset), (2048*31 - 512 * rx_buffer_preset)*sizeof(uint8_t));
			buf_index += 2048 * 31 - 512 * rx_buffer_preset;
			current_address += 31;
			remaining_block_count -= 31 * 4  - rx_buffer_preset;
			remaining_page_count -= 31;
		}
		rx_buffer_preset = 0;
	}
	flash_busy = 0;

	return 0;
}

//Write from buf from USB with blk_len blocks to blk_addr.
int flash_write(uint8_t* buf, uint32_t blk_addr, uint16_t blk_len) {
	uint16_t remaining_block_count = blk_len;
	uint16_t current_address = (blk_addr/4) & 0xFFFF;
	uint16_t current_block = (blk_addr/4) >> 6;
	uint16_t remaining_page_count = (blk_len + blk_addr - 1) / 4 - current_address + 1;
	uint16_t buf_index = 0;
	uint16_t page_offset = blk_addr % 4;
	uint16_t tx_buffer_preset = page_offset;
	uint8_t tx_buffer[2051] = {0}; //Data (2048 bytes) + Transmission (3 bytes)
	uint8_t read_buffer[2052] = {0};
	int current_flash_num;
//	uint8_t read_buffer[2048] = {0}; //Debug
//	for (int i = 0; i < 2048; i++) {
//		read_buffer[i] = 0xBB;
//	}
	Flash_Status error;

	//Return immediate of blk_len is 0
	if (blk_len == 0) {
		return 0;
	}

	//Check if we have enough data to transfer from a certain address and pointers are okay
	if ((blk_len + blk_addr) > 262144 || check_pointers() != FLASH_OK) {
		return -1;
	}
	//Returns Busy if flash freeze
	if (flash_freeze == 1) {
		return 1;
	}

	flash_busy = 1;


	//Loop until all data is written
	for (int i = 0; i < remaining_page_count; i++) {
		if (i == 0 || current_block != current_address >> 6) {
			current_block = current_address >> 6;
			flash_blk_manage(current_address);
			for (int j = current_block << 6; j < current_address; j++) {
				flash_write_from_erase(j);
			}
			if (tx_buffer_preset != 0) {
				current_flash_num = flash_chip_num;
				flash_chip_num = 4;
				error = flash_page_read(current_address);
				error = flash_data_read(1, read_buffer, 1);
				memcpy((void*)(tx_buffer + 3), (void*)(read_buffer + 4), tx_buffer_preset * 512 *sizeof(uint8_t));
				memcpy((void*)(tx_buffer + 3 + tx_buffer_preset * 512), (void*)buf, 512*(4 - tx_buffer_preset)*sizeof(uint8_t));
				flash_chip_num = current_flash_num;
				error = flash_data_write(0, 2048, tx_buffer, 1);

				if (error != FLASH_OK) {
					return -1;
					flash_busy = 0;
				}

				//Commit changes
				error = flash_page_write(current_address);

				if (error != FLASH_OK) {
					return -1;
					flash_busy = 0;
				}

				tx_buffer_preset = 0;
				buf_index += 2048 - 512 * tx_buffer_preset;
				current_address++;
				remaining_block_count -= 4 - tx_buffer_preset;
				continue;
			}
		}
		//Transfer remaining blocks
		if (i == remaining_page_count - 1) {
			if (remaining_block_count < 4) {
				break;
			}
			memcpy((void*)(tx_buffer + 3), (void*)(buf + buf_index), 512*remaining_block_count*sizeof(uint8_t));
			//Write Data out
			error = flash_data_write(512 * tx_buffer_preset, 512*remaining_block_count, tx_buffer, 0);

			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}

			//Commit changes
			error = flash_page_write(current_address);

			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}
			buf_index += remaining_block_count;
			current_address++;
			remaining_block_count = 0;

		}
		//Transfer max 31 blocks
		else {
			memcpy((void*)(tx_buffer + 3), (void*)(buf + buf_index), (2048 - tx_buffer_preset * 512)*sizeof(uint8_t));
			//Write Data out
			error = flash_data_write(512 * tx_buffer_preset, 2048 - tx_buffer_preset * 512, tx_buffer, 0);

			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}

			//Commit changes
			error = flash_page_write(current_address);

			if (error != FLASH_OK) {
				return -1;
				flash_busy = 0;
			}

			buf_index += 2048 - 512 * tx_buffer_preset;
			current_address++;
			remaining_block_count -= 4 - tx_buffer_preset;
		}
		tx_buffer_preset = 0;
	}

	if (remaining_block_count != 0) {
		memcpy((void*)(tx_buffer + 3), (void*)(buf + buf_index), 512*remaining_block_count*sizeof(uint8_t));
		current_flash_num = flash_chip_num;
		flash_chip_num = 4;
		error = flash_page_read(current_address);
		error = flash_data_read(1, read_buffer, 1);
		flash_chip_num = current_flash_num;
		memcpy((void*)(tx_buffer + 3 + 512*remaining_block_count), (void*)(read_buffer + 4), 512*(4 - remaining_block_count)*sizeof(uint8_t));
		error = flash_data_write(0, 2048, tx_buffer, 1);

		if (error != FLASH_OK) {
			return -1;
			flash_busy = 0;
		}

		//Commit changes
		error = flash_page_write(current_address);

		if (error != FLASH_OK) {
			return -1;
			flash_busy = 0;
		}
		current_address++;
	}
	while (current_block != current_address >> 6) {
		flash_write_from_erase(current_address);
		current_address++;
	}

	flash_busy = 0;
//	flash_read(read_buffer, current_address, blk_len);
	return 0;
}

//To erase and save data to buffer chip
void flash_blk_manage(uint16_t page_address) {
	uint16_t block_address = page_address & 0b000000;
	int current_flash_num = flash_chip_num;
	uint8_t tx_buffer[2051] = {0};
	Flash_Status error;
	flash_chip_num = 4;
	flash_block_erase(0);
	flash_chip_num = current_flash_num;
	for (int j = 0; j < 3; j++) {
		error = flash_page_read(block_address);
		if (j == 2) {
			error = flash_data_read(2, rx_buffer_full_size, 1);
		}
		else {
			error = flash_data_read(31, rx_buffer_full_size, 1);
		}
		flash_chip_num = 4;
		for (int i = 0; i < 31; i++) {
			if (j == 2 && i == 2) {
				break;
			}
			memcpy((void*)(tx_buffer + 3), (void*)(rx_buffer_full_size + 4 + i * 2048), 2048*sizeof(uint8_t));
			error = flash_data_write(0, 2048, tx_buffer, 1);
			error = flash_page_write(block_address);
			block_address++;
		}
		flash_chip_num = current_flash_num;
	}
	flash_block_erase(page_address);
}

//To rewrite data from the buffer chip
void flash_write_from_erase(uint16_t page_address) {
	uint16_t other_chip_address = page_address && 0b111111;
	int current_flash_num = flash_chip_num;
	Flash_Status error;
	uint8_t rx_buffer[2052] = {0};
	uint8_t tx_buffer[2051] = {0};
	flash_chip_num = 4;
	error = flash_page_read(other_chip_address);
	error = flash_data_read(1, rx_buffer, 1);
	memcpy((void*)(tx_buffer + 3), (void*)(rx_buffer + 4), 2048*sizeof(uint8_t));
	flash_chip_num = current_flash_num;
	error = flash_data_write(0, 2048, tx_buffer, 1);
	error = flash_page_write(page_address);
}

//USB Verification Functions

//Checks if pointers have been passed in to flash_init()
Flash_Status check_pointers() {
	if (hspiflash == NULL) {
		return FLASH_SPI_UNINITIALIZED;
	}
	return FLASH_OK;
}

//Check if chip is busy, returns -1 if error, 0 if not busy, 1 if busy
int check_busy() {
	uint8_t value;
	Flash_Status error_one = flash_read_status_register(3, &value);

	if (error_one != FLASH_OK) {
		return -1;
	}
	else if ((value & 0x1) != 0) {
		return 1;
	}
	return 0;
}



//Setters and Getters for parameters

//Setters of flash_chip_num are expected to ensure flash is not busy.
//Ideally you freeze then wait for processing to finish then change flash chip.
void set_flash_chip_num(int value) {
	flash_chip_num = value;
}
void set_flash_freeze(int value) {
	if (value != 0) {
		flash_freeze = 1;
	}
	else {
		flash_freeze = 0;
	}
}
int get_flash_chip_num() {
	return flash_chip_num;
}
int get_flash_freeze() {
	return flash_freeze;
}
int get_flash_busy() {
	return flash_busy;
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
	
	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	int busy = 1;

	while (busy != 0) {
		busy = check_busy();
		if (busy == -1) {
			return FLASH_BUSY_ERROR;
		}
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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, size, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	int busy = 1;

	while (busy != 0) {
		busy = check_busy();
		if (busy == -1) {
			return FLASH_BUSY_ERROR;
		}
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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	pin_setup(flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, bytes + 3, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	tx_wel_buffer[0] = FLASH_WRITE_DISABLE;

	pin_setup(flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	return FLASH_OK;
}

//Commits data write changes.
Flash_Status flash_page_write(uint16_t page_address) {
	Flash_Status pointer_check = check_pointers();
	uint8_t data_out;
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	uint8_t tx_buffer[4] = {FLASH_PROGRAM_EXECUTE, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;

	pin_setup(flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	int busy = 1;

	while (busy != 0) {
		busy = check_busy();
		if (busy == -1) {
			return FLASH_BUSY_ERROR;
		}
	}

	return FLASH_OK;
}

//Erases 128KB (1 block or 64 pages) addressed by PA[15:6]
Flash_Status flash_block_erase(uint16_t page_address) {
	Flash_Status pointer_check = check_pointers();
	if (pointer_check != FLASH_OK) {
		return pointer_check;
	}

	uint8_t tx_wel_buffer[1] = {FLASH_WRITE_ENABLE};

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_one != FLASH_OK) {
		return error_one;
	}
	else if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	uint8_t tx_buffer[4] = {FLASH_BLOCK_ERASE, 0x00, 0x00, 0x00};
	tx_buffer[2] = page_address >> 8;
	tx_buffer[3] = page_address & 0xff;

	pin_setup(flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 4, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	int busy = 1;

	while (busy != 0) {
		busy = check_busy();
		if (busy == -1) {
			return FLASH_BUSY_ERROR;
		}
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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 3, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 3, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 4, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_Transmit(hspiflash, tx_wel_buffer, 1, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	pin_setup(flash_chip_num, 0, -1, -1);
	error_two = HAL_SPI_Transmit(hspiflash, tx_buffer, 5, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

	if (error_two != 0) {
		return FLASH_HAL_ERROR;
	}

	int busy = 1;

	while (busy != 0) {
		busy = check_busy();
		if (busy == -1) {
			return FLASH_BUSY_ERROR;
		}
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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 82, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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

	Flash_Status error_one = pin_setup(flash_chip_num, 0, -1, -1);
	int error_two = HAL_SPI_TransmitReceive(hspiflash, tx_buffer, rx_buffer, 5, 1000);
	pin_setup(flash_chip_num, 1, -1, -1);

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
