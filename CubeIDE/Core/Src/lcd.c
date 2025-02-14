/*
 * lcd.c
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */

#include "lcd.h"
#include "stm32f4xx_hal_spi.h"

void periodic_send() {
	HAL_SPI_Transmit(&hspi1, TX_Buffer, 1, 1000); //Sending in Blocking mode
	HAL_Delay(100);
}




