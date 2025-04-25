/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "usb_device.h"
#include "usbd_storage_if.h"
#include "usbd_core.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "keypad.h"
#include "lcd.h"
#include "fingerprint_sensor.h"
#include "flash.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart4;

/* USER CODE BEGIN PV */
//(update with LCD)

//FLASH GPIOs
const IC_Pin FLASH_P_HOLD_ONE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_6};
const IC_Pin FLASH_P_HOLD_TWO = (IC_Pin){.pin_letter = GPIOA, .pin_num = GPIO_PIN_2};
const IC_Pin FLASH_P_HOLD_THREE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_1};
const IC_Pin FLASH_P_HOLD_FOUR = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_6};
const IC_Pin FLASH_P_CS_ONE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_9};
const IC_Pin FLASH_P_CS_TWO = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_1};
const IC_Pin FLASH_P_CS_THREE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_3};
const IC_Pin FLASH_P_CS_FOUR = (IC_Pin){.pin_letter = GPIOH, .pin_num = GPIO_PIN_1};
const IC_Pin FLASH_P_WP_ONE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_7};
const IC_Pin FLASH_P_WP_TWO = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_0};
const IC_Pin FLASH_P_WP_THREE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_2};
const IC_Pin FLASH_P_WP_FOUR = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_0};


//KEYPAD GPIOs
const IC_Pin KEY_P_C_ONE = (IC_Pin){.pin_letter = GPIOA, .pin_num = GPIO_PIN_15};
const IC_Pin KEY_P_C_TWO = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_10};
const IC_Pin KEY_P_C_THREE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_11};
const IC_Pin KEY_P_R_ONE = (IC_Pin){.pin_letter = GPIOC, .pin_num = GPIO_PIN_12};
const IC_Pin KEY_P_R_TWO = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_3};
const IC_Pin KEY_P_R_THREE = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_4};
const IC_Pin KEY_P_R_FOUR = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_5};

//LCD GPIOs
const IC_Pin LCD_P_CS = (IC_Pin){.pin_letter = GPIOB, .pin_num = GPIO_PIN_12};

//DEBUG GPIOs
const IC_Pin DEBUG_P_NINE = (IC_Pin){.pin_letter = GPIOA, .pin_num = GPIO_PIN_9};
const IC_Pin DEBUG_P_EIGHT = (IC_Pin){.pin_letter = GPIOA, .pin_num = GPIO_PIN_8};

/* USER CODE END PV */

// STATES
enum State {
	USERSEL,
	RECVPASS,
	PASSWORD,
	FINGERPRINT,
	UNLOCKED,
	CHGPRINT,
	CHGPIN,
	LOCKING
};

struct User {
	int user_id;
	uint8_t password[4];
};

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_UART4_Init(void);
static void MX_SPI1_Init(void);
static void MX_SPI2_Init(void);
static void MX_TIM7_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
const struct User default_users[4] = {
		(struct User){.user_id = 1, .password = {0, 0, 0, 0}},
		(struct User){.user_id = 2, .password = {0, 0, 0, 0}},
		(struct User){.user_id = 3, .password = {0, 0, 0, 0}},
		(struct User){.user_id = 4, .password = {0, 0, 0, 0}}
};

struct User users[4];

enum State state;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */
  /* USER CODE END 1 */

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  MX_GPIO_Init();
  MX_SPI1_Init();
  uint8_t value;
  Init_Pin();
  flash_init(&hspi1);
  for (int i = 0; i < 4; i++) {
	  set_flash_chip_num(i);
	  flash_read_status_register(1, &value);
	  flash_write_status_register(1, value & 0b10000111);
	  flash_read_status_register(2, &value);
	  flash_write_status_register(2, value & 0b11110111);
  }

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_UART4_Init();
  MX_SPI2_Init();
  MX_USB_DEVICE_Init();
  MX_TIM7_Init();
  /* USER CODE BEGIN 2 */
  HAL_TIM_Base_Start_IT(&htim7);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

//  enum State state = UNLOCKED;

  switch_to_usersel();

  load_users_from_flash();

//  struct User users[4] = {user_1, user_2, user_3, user_4};

//  struct User* selected_user = NULL;
//  struct User* selected_user = &user_1;
  struct User* selected_user = &users[0];

  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  switch (state) {
          case USERSEL: {
              if (is_key_pressed(0, 0, 1)) { // 1
                  selected_user = &users[0];
                  switch_to_recvpass();
              } else if (is_key_pressed(0, 1, 1)) { // 2
                  selected_user = &users[1];
                  switch_to_recvpass();
              } else if (is_key_pressed(0, 2, 1)) { // 3
                  selected_user = &users[2];
                  switch_to_recvpass();
              } else if (is_key_pressed(1, 0, 1)) { // 4
                  selected_user = &users[3];
                  switch_to_recvpass();
              }
              break;
          }

          case RECVPASS: {
              if (is_key_pressed(3, 1, 1)) { // 0 held for 4 sec
                  switch_to_usersel();
              }
              else if (is_key_pressed(3, 0, 1)) { // star - SELECTED PASSWORD
            	  switch_to_password();
              }
              else if (is_key_pressed(3, 2, 1)) { // pound - SELECTED FINGERPRINT
            	  switch_to_fingerprint();
              }
              break;
          }

          case PASSWORD: {
              int success = 1;
              int count = 0;
              while (count < 4) {
                  char key = '\0';
                  for (int i = 0; i < NUM_ROWS; i++) {
                	  for (int j = 0; j < NUM_COLS; j++) {
                		  if (is_key_pressed(i, j, 1)) {
                			  key = key_to_char(i, j);
                		  }
                	  }
                  }
                  // await key press
                  if (key != '\0' && key != '*' && key != '#') {
                	  if (key != selected_user->password[count] + '0') {
                		  success = 0;
                	  }
                	  count++;
                  }
              }
              if (success) {
            	  // TODO: COPY FLASH DATA TO BUFFER HERE
            	  set_flash_chip_num(selected_user->user_id);
            	  flash_read(buffer, 0, 64);

            	  if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
            	  {
            	      Error_Handler();
            	  }

            	  switch_to_unlocked();
              }
              else {
                  Write_Pin(LCD_P_CS, 0);
                  lcd_clear(&hspi2);
                  lcd_cursor_location(&hspi2, 0x00); // first row of LCD
                  lcd_print(&hspi2, (uint8_t*)"INCORRECT");
                  lcd_cursor_location(&hspi2, 0x40);
                  lcd_print(&hspi2, (uint8_t*)"PASSWORD");
                  Write_Pin(LCD_P_CS, 1);
                  HAL_Delay(2000);

                  switch_to_recvpass();
              }
              break;
          }

          case FINGERPRINT: {
              // send fingerprint match request
              uint8_t ack_type = 9;
              compare_1_1(selected_user->user_id, &ack_type);
              if (ack_type == ACK_SUCCESS) {
            	  // TODO: COPY FLASH DATA TO BUFFER HERE
            	  set_flash_chip_num(selected_user->user_id);
            	  flash_read(buffer, 0, 64);
                  if (USBD_Start(&hUsbDeviceHS) != USBD_OK)
                  {
                	  Error_Handler();
                  }

                  switch_to_unlocked();
              }
              else {
                  Write_Pin(LCD_P_CS, 0);
                  lcd_clear(&hspi2);
                  lcd_cursor_location(&hspi2, 0x00); // first row of LCD
                  lcd_print(&hspi2, (uint8_t*)"INCORRECT");
                  lcd_cursor_location(&hspi2, 0x40);
                  lcd_print(&hspi2, (uint8_t*)"PRINT");
                  Write_Pin(LCD_P_CS, 1);
                  HAL_Delay(2000);

                  switch_to_recvpass();
              }
              break;
          }
              
          case UNLOCKED: {
              if (is_key_pressed(3, 1, 1)) { // 0 pressed
		          switch_to_chgprint();
		      }
		      else if (is_key_pressed(3, 0, 1)) { // * pressed
		          switch_to_chgpin();
		      }
		      else if (is_key_pressed(3, 2, 1)) { // # pressed
		    	  switch_to_locking();
		      }
              break;
          }

          case CHGPRINT: {
		      // ask user for print on fingerprint sensor
		      uint8_t ack_type = 9;
		      delete_specified_user(selected_user->user_id, &ack_type);
		      for (int i = 1; i <= 3; i++) {
		          add_fingerprint(i, selected_user->user_id, 1, &ack_type);
		          if (ack_type != ACK_SUCCESS) {
		        	  Write_Pin(LCD_P_CS, 0);
		        	  lcd_clear(&hspi2);
		        	  lcd_cursor_location(&hspi2, 0x00); // first row of LCD
		        	  lcd_print(&hspi2, (uint8_t*)"BAD READING");
		        	  lcd_cursor_location(&hspi2, 0x40);
		        	  lcd_print(&hspi2, (uint8_t*)"ABORTING");
		        	  Write_Pin(LCD_P_CS, 1);
		        	  HAL_Delay(2000);
		        	  break;
		          }
		      }
		      switch_to_unlocked();
		      break;
          }

          case CHGPIN: {
		      int success = 1;
		      int count = 0;
		      int new_password[4];
		      while (count < 4) {
		          char key = '\0';
		          for (int i = 0; i < NUM_ROWS; i++) {
		        	  for (int j = 0; j < NUM_COLS; j++) {
		        		  if (is_key_pressed(i, j, 1)) {
		        			  key = key_to_char(i, j);
		        		  }
		        	  }
		          }
		          // await key press
		          if (key == '#') { // cancel
		        	  success = 0;
		        	  break;
		          }

		          if (key != '\0') { // password entering
		        	  new_password[count] = key - '0';
		        	  count++;
		          }
		      }

		      // Change the password if it's good
		      if (success) {
		          for (int i = 0; i < 4; i++) {
		        	  selected_user->password[i] = new_password[i];
		        	  users[(selected_user->user_id) - 1].password[i] = new_password[i];
		          }
//		          memcpy(users[(selected_user->user_id) - 1].password, new_password, 4);
		          save_users_to_flash();
		      }

		      switch_to_unlocked();
		      break;
          }

          case LOCKING: {
              if (is_key_pressed(3, 2, 1)) { // # pressed
            	  // TODO: copy and paste buffer to flash IC
            	  set_flash_chip_num(selected_user->user_id);
            	  flash_write(buffer, 0, 64);
            	  if (USBD_Stop(&hUsbDeviceHS) != USBD_OK) {
            	      Error_Handler();
            	  }
		          switch_to_usersel();
		      } else if (is_key_pressed(3, 0, 1)) { // * pressed
		          switch_to_unlocked();
		      }
              break;
          }
	  }
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /*AXI clock gating */
  RCC->CKGAENR = 0xFFFFFFFF;

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = 64;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 8;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x0;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x0;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.TxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.RxCRCInitializationPattern = SPI_CRC_INITIALIZATION_ALL_ZERO_PATTERN;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 31999;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 1;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief UART4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART4_Init(void)
{

  /* USER CODE BEGIN UART4_Init 0 */

  /* USER CODE END UART4_Init 0 */

  /* USER CODE BEGIN UART4_Init 1 */

  /* USER CODE END UART4_Init 1 */
  huart4.Instance = UART4;
  huart4.Init.BaudRate = 19200;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart4, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart4, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN UART4_Init 2 */

  /* USER CODE END UART4_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_1, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : PH1 */
  GPIO_InitStruct.Pin = GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC6 PC7 PC9 PC12 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_9|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA2 PA3 PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB12 PB3
                           PB4 PB5 PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_12|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC10 PC11 */
  GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
//Easy Write pin with the ic_pin type

#define USER_SECTOR_ADDR 0X0817E000
#define USER_SECTOR_NUM  63

void load_users_from_flash(void) {
	// read user0 and user1
	memcpy(&users[0], (void*)USER_SECTOR_ADDR, 16);

	// read user2 and user3
	memcpy(&users[2], (void*)(USER_SECTOR_ADDR + 16), 16);
    if (users[0].user_id != 1 || users[1].user_id != 2 || users[2].user_id != 3  || users[3].user_id != 4) {
        memcpy(users, default_users, sizeof(users));
    }
}

void save_users_to_flash(void) {
    HAL_FLASH_Unlock();

    // erase the sector before writing
    FLASH_EraseInitTypeDef eraseInit;
    eraseInit.TypeErase    = FLASH_TYPEERASE_SECTORS;
    eraseInit.Banks        = FLASH_BANK_2;
    eraseInit.Sector       = USER_SECTOR_NUM;
    eraseInit.NbSectors    = 1;
    uint32_t sectorError = 0;
    HAL_FLASHEx_Erase(&eraseInit, &sectorError);

    uint8_t temp1[16] __attribute__((aligned(16)));
    memset(temp1, 0xFF, 16);
    memcpy(temp1, &users[0], 16); // users[0] and users[1]
    HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_FLASHWORD,
        USER_SECTOR_ADDR,
        (uint32_t)temp1
    );

    uint8_t temp2[16] __attribute__((aligned(16)));
    memset(temp2, 0xFF, 16);
    memcpy(temp2, &users[2], 16); // users[2] and users[3]
    HAL_FLASH_Program(
        FLASH_TYPEPROGRAM_FLASHWORD,
        USER_SECTOR_ADDR + 16,
        (uint32_t)temp2
    );

    HAL_FLASH_Lock();
}

void switch_to_usersel() {
	state = USERSEL;

	Write_Pin(LCD_P_CS, 0);
	lcd_on(&hspi2); // this line
	lcd_light(&hspi2, 0x8); // and this line might only be needed on power up
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // 1st row LCD
	lcd_print(&hspi2, (uint8_t*)"Select User ID");
	lcd_cursor_location(&hspi2, 0x40); // second row of LCD
	lcd_print(&hspi2, (uint8_t*)"1 2 3 4");
	Write_Pin(LCD_P_CS, 1);
}

void switch_to_recvpass() {
	state = RECVPASS;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"SELECT 1 CHOICE");
	lcd_cursor_location(&hspi2, 0x40);
	lcd_print(&hspi2, (uint8_t*)"FP # PASS * BK 0");
	Write_Pin(LCD_P_CS, 1);
}

void switch_to_password() {
	state = PASSWORD;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"ENTER PASSWORD");
    Write_Pin(LCD_P_CS, 1);
}

void switch_to_fingerprint() {
	state = FINGERPRINT;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"ENTER FP");
	Write_Pin(LCD_P_CS, 1);
}

void switch_to_unlocked() {
	state = UNLOCKED;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"UNLOCKED");
	lcd_cursor_location(&hspi2, 0x40);
	lcd_print(&hspi2, (uint8_t*)"FP 0 PASS * LK #");
	Write_Pin(LCD_P_CS, 1);
}

void switch_to_chgprint() {
	state = CHGPRINT;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"ENTER PRINT");
	Write_Pin(LCD_P_CS, 1);
//	HAL_Delay(2000);
}

void switch_to_chgpin() {
	state = CHGPIN;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"ENTER PASS");
	lcd_cursor_location(&hspi2, 0x40);
	lcd_print(&hspi2, (uint8_t*)"OR # TO CANCEL");
	Write_Pin(LCD_P_CS, 1);
}

void switch_to_locking() {
	state = LOCKING;

	Write_Pin(LCD_P_CS, 0);
	lcd_clear(&hspi2);
	lcd_cursor_location(&hspi2, 0x00); // first row of LCD
	lcd_print(&hspi2, (uint8_t*)"LOCK & SAVE?");
	lcd_cursor_location(&hspi2, 0x40);
	lcd_print(&hspi2, (uint8_t*)"Y:# N:*");
	Write_Pin(LCD_P_CS, 1);
}

void Write_Pin(IC_Pin pin, int value)
{
	if (value == 0) {
		HAL_GPIO_WritePin(pin.pin_letter, pin.pin_num, GPIO_PIN_RESET);
	}
	else {
		HAL_GPIO_WritePin(pin.pin_letter, pin.pin_num, GPIO_PIN_SET);
	}

}

int Read_Pin(IC_Pin pin)
{
	GPIO_PinState value = HAL_GPIO_ReadPin(pin.pin_letter, pin.pin_num);

	if (value == GPIO_PIN_SET){
		return 1;
	}
	return 0;
}

//Initialize pins to "default value" (update with LCD)
void Init_Pin() {
	Write_Pin(FLASH_P_WP_ONE, 1);
	Write_Pin(FLASH_P_WP_TWO, 1);
	Write_Pin(FLASH_P_WP_THREE, 1);
	Write_Pin(FLASH_P_WP_FOUR, 1);
	Write_Pin(FLASH_P_CS_ONE, 1);
	Write_Pin(FLASH_P_CS_TWO, 1);
	Write_Pin(FLASH_P_CS_THREE, 1);
	Write_Pin(FLASH_P_CS_FOUR, 1);
	Write_Pin(FLASH_P_HOLD_ONE, 1);
	Write_Pin(FLASH_P_HOLD_TWO, 1);
	Write_Pin(FLASH_P_HOLD_THREE, 1);
	Write_Pin(FLASH_P_HOLD_FOUR, 1);
	Write_Pin(DEBUG_P_EIGHT, 0);
	Write_Pin(DEBUG_P_NINE, 0);
}
/* USER CODE END 4 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
