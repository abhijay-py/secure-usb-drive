/*
 * lcd.h
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal_spi.h"

uint8_t TX_Buffer [] = "A";

void periodic_send();

#endif /* INC_LCD_H_ */
