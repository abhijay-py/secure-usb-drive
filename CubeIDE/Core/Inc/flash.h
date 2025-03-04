/*
 * flash.h
 *
 *  Created on: Mar 3, 2025
 *      Author: brand
 */

#ifndef INC_FLASH_H_
#define INC_FLASH_H_

void reset_ic(SPI_HandleTypeDef *hspi1, int flash_chip_num);
void flash_read_jedec_id(SPI_HandleTypeDef *hspi1, int flash_chip_num);

#endif /* INC_FLASH_H_ */
