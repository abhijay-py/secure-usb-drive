/*
 * lcd.c
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */
#include "main.h"
#include "lcd.h"
#include "stm32f4xx_hal_spi.h"

void periodic_send(SPI_HandleTypeDef *hspi1) {

	uint8_t TX_Buffer [] = "A" ;

	HAL_SPI_Transmit(hspi1, TX_Buffer, 1, 1000); //Sending in DMA mode
	HAL_Delay(100);
}




