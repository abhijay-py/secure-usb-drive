/*
 * flash.h
 *
 *  Created on: Mar 3, 2025
 *      Author: brand
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

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

} Flash_Status;

//Raw data handling functions
Flash_Status reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num);
Flash_Status flash_page_read(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t page_address);
Flash_Status flash_data_read(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t bytes_or_pages, uint8_t* rx_buffer, int count_in_pages);
Flash_Status flash_data_write(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t column_address, uint16_t bytes, uint8_t* tx_buffer, int overwrite_other_data);
Flash_Status flash_page_write(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t page_address);
Flash_Status block_erase(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t page_address);
Flash_Status flash_read_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register, uint8_t* data_out);
Flash_Status flash_write_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register, uint8_t value);
Flash_Status flash_ecc_failure_address(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t* address_out);
Flash_Status flash_remap_bad_block(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t bad_block_address, uint16_t new_block_address);
Flash_Status flash_read_bad_block_LUT(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint8_t* rx_buffer);
Flash_Status pin_setup(int flash_chip_num, int cs, int wp, int hold);
Flash_Status flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num, int debug);

#endif /* INC_FLASH_H_ */
