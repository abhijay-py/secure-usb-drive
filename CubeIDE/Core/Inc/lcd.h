/*
 * lcd.h
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal_spi.h"

void lcd_init(SPI_HandleTypeDef *hspi1);
void lcd_cursor_on(SPI_HandleTypeDef *hspi1);
void lcd_display_first_row(SPI_HandleTypeDef *hspi1, uint8_t *string);
void lcd_display_second_row(SPI_HandleTypeDef *hspi1, uint8_t *string);
void periodic_send(SPI_HandleTypeDef *hspi1);

void send_string(SPI_HandleTypeDef *hspi1, uint8_t *string);
void send_byte(SPI_HandleTypeDef *hspi1, const uint8_t *byte);

#endif /* INC_LCD_H_ */
