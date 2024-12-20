/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
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

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define FSW_LED4_Pin GPIO_PIN_9
#define FSW_LED4_GPIO_Port GPIOC
#define FSW_LED3_Pin GPIO_PIN_8
#define FSW_LED3_GPIO_Port GPIOA
#define FSW_LED2_Pin GPIO_PIN_9
#define FSW_LED2_GPIO_Port GPIOA
#define FSW_LED1_Pin GPIO_PIN_10
#define FSW_LED1_GPIO_Port GPIOA
#define FSW_BTN1_Pin GPIO_PIN_10
#define FSW_BTN1_GPIO_Port GPIOC
#define FSW_BTN1_EXTI_IRQn EXTI15_10_IRQn
#define FSW_BTN2_Pin GPIO_PIN_11
#define FSW_BTN2_GPIO_Port GPIOC
#define FSW_BTN2_EXTI_IRQn EXTI15_10_IRQn
#define FSW_BTN3_Pin GPIO_PIN_12
#define FSW_BTN3_GPIO_Port GPIOC
#define FSW_BTN3_EXTI_IRQn EXTI15_10_IRQn
#define FSW_BTN4_Pin GPIO_PIN_2
#define FSW_BTN4_GPIO_Port GPIOD
#define FSW_BTN4_EXTI_IRQn EXTI2_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
