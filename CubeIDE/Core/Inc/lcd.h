/*
 * lcd.h
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal_spi.h"

void periodic_send(SPI_HandleTypeDef *hspi1);

#endif /* INC_LCD_H_ */
