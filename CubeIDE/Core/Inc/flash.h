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

void pin_setup(int flash_chip_num, int cs, int wp, int hold);
void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num);
void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num, int debug);
uint8_t flash_read_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register);
void flash_write_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register, uint8_t value);
void flash_page_transfer(SPI_HandleTypeDef *hspi1, int flash_chip_num, uint16_t page_address);


#endif /* INC_FLASH_H_ */
