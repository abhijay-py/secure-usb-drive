/*
 * flash.h
 *
 *  Created on: Mar 3, 2025
 *      Author: brand
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_spi.h"

typedef struct
{
 	GPIO_TypeDef* pin_letter;
	uint16_t pin_num;
}IC_Pin;

void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num);
void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num);
uint8_t flash_read_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register);
void flash_write_status_register(SPI_HandleTypeDef *hspi1, int flash_chip_num, int status_register);

extern const IC_Pin FLASH_P_HOLD;
extern const IC_Pin FLASH_P_CHIP_SELECT;
extern const IC_Pin FLASH_P_WRITE_PROTECT;

extern const uint8_t FLASH_RESET;
extern const uint8_t FLASH_WRITE_ENABLE;
extern const uint8_t FLASH_WRITE_DISABLE;
extern const uint8_t FLASH_PAGE_READ;
extern const uint8_t FLASH_READ_FROM_CACHE;
extern const uint8_t FLASH_PROGRAM_EXECUTE;
extern const uint8_t FLASH_PROGRAM_LOAD;
extern const uint8_t FLASH_PROGRAM_LOAD_RANDOM;
extern const uint8_t FLASH_BLOCK_ERASE;
extern const uint8_t FLASH_READ_STATUS_REGISTER;
extern const uint8_t FLASH_WRITE_STATUS_REGISTER;
extern const uint8_t FLASH_READ_JEDEC_ID[5];


#endif /* INC_FLASH_H_ */
