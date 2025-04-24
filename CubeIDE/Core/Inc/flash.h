/*
 * flash.h
 *
 *  Created on: Mar 3, 2025
 *      Author: brand
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include <string.h>

#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include "main.h"

typedef enum {
	FLASH_OK,
	FLASH_HAL_ERROR,
	FLASH_INVALID_CHIP_NUM,
	FLASH_INVALID_STATUS_REG,
	FLASH_INVALID_READ_SIZE,
	FLASH_INVALID_WRITE_SIZE,
	FLASH_SPI_UNINITIALIZED,
	FLASH_BUSY_ERROR,
} Flash_Status;

//USB Data Wrapper Functions
void flash_init(SPI_HandleTypeDef *hspi1);
int flash_read(uint8_t* buf, uint32_t blk_addr, uint16_t blk_len);
int flash_write(uint8_t* buf, uint32_t blk_addr, uint16_t blk_len);
void flash_blk_manage(uint16_t page_address);
void flash_write_from_erase(uint16_t page_address);
//USB Verification Functions
Flash_Status check_pointers();
int check_busy();

//Setters and Getters for parameters
void set_flash_chip_num(int value);
void set_flash_freeze(int value);
int get_flash_chip_num();
int get_flash_freeze();
int get_flash_busy();

//Raw data handling functions
Flash_Status reset_ic(int reset_num);
Flash_Status flash_page_read(uint16_t page_address);
Flash_Status flash_data_read(uint16_t bytes_or_pages, uint8_t* rx_buffer, int count_in_pages);
Flash_Status flash_data_write(uint16_t column_address, uint16_t bytes, uint8_t* tx_buffer, int overwrite_other_data);
Flash_Status flash_page_write(uint16_t page_address);
Flash_Status flash_block_erase(uint16_t page_address);
Flash_Status flash_read_status_register(int status_register, uint8_t* data_out);
Flash_Status flash_write_status_register(int status_register, uint8_t value);
Flash_Status flash_ecc_failure_address(uint16_t* address_out);
Flash_Status flash_remap_bad_block(uint16_t bad_block_address, uint16_t new_block_address);
Flash_Status flash_read_bad_block_LUT(uint8_t* rx_buffer);
Flash_Status pin_setup(int chip_num, int cs, int wp, int hold);
Flash_Status flash_read_jedec_id(int debug);


#endif /* INC_FLASH_H_ */
