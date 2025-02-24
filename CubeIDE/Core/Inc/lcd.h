/*
 * lcd.h
 *
 *  Created on: Feb 12, 2025
 *      Author: brand
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include "stm32f4xx_hal_spi.h"

#define HOME 0x46

void lcd_welcome(SPI_HandleTypeDef *hspi2);
void lcd_clear(SPI_HandleTypeDef *hspi2);
void lcd_on(SPI_HandleTypeDef *hspi2);
void lcd_off(SPI_HandleTypeDef *hspi2);
void lcd_cursor_on(SPI_HandleTypeDef *hspi2);
void lcd_display_first_row(SPI_HandleTypeDef *hspi2, uint8_t *string);
void lcd_display_second_row(SPI_HandleTypeDef *hspi2, uint8_t *string);
void periodic_send(SPI_HandleTypeDef *hspi2);

//void send_string(SPI_HandleTypeDef *hspi2, uint8_t *string);
void send_bytes(SPI_HandleTypeDef *hspi2, const uint8_t *tx_buffer, size_t size);

#endif /* INC_LCD_H_ */
