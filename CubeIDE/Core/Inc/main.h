/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
typedef struct
{
 	GPIO_TypeDef* pin_letter;
	uint16_t pin_num;
}IC_Pin;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
extern TIM_HandleTypeDef htim7;
extern UART_HandleTypeDef huart4;

//FLASH GPIOs
extern const IC_Pin FLASH_P_HOLD_ONE;
extern const IC_Pin FLASH_P_HOLD_TWO;
extern const IC_Pin FLASH_P_HOLD_THREE;
extern const IC_Pin FLASH_P_HOLD_FOUR;
extern const IC_Pin FLASH_P_CS_ONE;
extern const IC_Pin FLASH_P_CS_TWO;
extern const IC_Pin FLASH_P_CS_THREE;
extern const IC_Pin FLASH_P_CS_FOUR;
extern const IC_Pin FLASH_P_WP_ONE;
extern const IC_Pin FLASH_P_WP_TWO;
extern const IC_Pin FLASH_P_WP_THREE;
extern const IC_Pin FLASH_P_WP_FOUR;


//KEYPAD GPIOs
extern const IC_Pin KEY_P_C_ONE;
extern const IC_Pin KEY_P_C_TWO;
extern const IC_Pin KEY_P_C_THREE;
extern const IC_Pin KEY_P_R_ONE;
extern const IC_Pin KEY_P_R_TWO;
extern const IC_Pin KEY_P_R_THREE;
extern const IC_Pin KEY_P_R_FOUR;

//LCD GPIOs

extern const IC_Pin LCD_P_CS;

//DEBUG GPIOs
extern const IC_Pin DEBUG_P_NINE;
extern const IC_Pin DEBUG_P_EIGHT;



/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void Write_Pin(IC_Pin pin, int value);
int Read_Pin(IC_Pin pin);
void init_pin();
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
