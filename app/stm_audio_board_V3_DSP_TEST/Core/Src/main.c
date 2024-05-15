/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "octospi.h"
#include "sai.h"
#include "spi.h"
#include "usb_device.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal.h"
#include "AD1939_driver.h"
#include "usbd_core.h"
//#include "usbd_audio.h"
#include "usbd_desc.h"
//#include "usbd_audio_if.h"
#include "usb_device.h"
#include "string.h"
#include <math.h>
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

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */



// USB AUDIO DEVICE TEST CODE START
#define AUDIO_FREQ 48000
#define AUDIO_BLOCK_SIZE 48  // block size in samples
#define PI 3.14159265358979f

//static int16_t audio_buffer[AUDIO_BLOCK_SIZE];
//void AUDIO_Init(void)
//{
//    for (int i = 0; i < AUDIO_BLOCK_SIZE; i++) {
//        audio_buffer[i] = (int16_t)(32760 * sin(2 * PI * 440 * i / AUDIO_FREQ));
//    }
//}
//
//int8_t AUDIO_Transmit_FS(uint8_t* Buf, uint32_t *Len)
//{
//    memcpy(Buf, audio_buffer, AUDIO_BLOCK_SIZE * sizeof(int16_t));
//    *Len = AUDIO_BLOCK_SIZE * sizeof(int16_t);
//    return USBD_OK;
//}
// USB AUDIO DEVICE TEST CODE END

volatile uint8_t 			ADC_HALF_COMPLETE_FLAG = 0;
volatile uint8_t 			DAC_HALF_COMPLETE_FLAG = 0;
volatile uint32_t input_i2s_buffer_au32[16];
volatile uint32_t output_i2s_buffer_au32[16];

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 0;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));

}
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 1;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
		SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 0;

	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
	output_i2s_buffer_au32[14] = input_i2s_buffer_au32[10];
	output_i2s_buffer_au32[15] = input_i2s_buffer_au32[11];

	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
}
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 1;


	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
	output_i2s_buffer_au32[6] = input_i2s_buffer_au32[2];
	output_i2s_buffer_au32[7] = input_i2s_buffer_au32[3];

	SCB_CleanDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));


}



#define SDRAM_ADDRESS_START 0xC0000000
#define SDRAM_SIZE 			0x100000 // 16Mb = 2MB

#define ARRAY_SIZE 10
// Define a padding variable to offset the array by 2 bytes
__attribute__((section(".sdram_section"))) volatile uint32_t sdram_array[ARRAY_SIZE];
__attribute__((section(".sdram_section"))) volatile uint16_t sdram_byte;

/*
 * This function puts the device into DFU (bootloader mode)
 * After you entered this, you can update FW via USB.
 */
void JumpToBootloader(void)
{
	  uint32_t i=0;
	  void (*SysMemBootJump)(void);

	  /* Set the address of the entry point to bootloader */
	     volatile uint32_t BootAddr = 0x1FF09800;

	  /* Disable all interrupts */
	     __disable_irq();

	  /* Disable Systick timer */
	     SysTick->CTRL = 0;

	  /* Set the clock to the default state */
	     HAL_RCC_DeInit();

	  /* Clear Interrupt Enable Register & Interrupt Pending Register */
	     for (i=0;i<5;i++)
	     {
		  NVIC->ICER[i]=0xFFFFFFFF;
		  NVIC->ICPR[i]=0xFFFFFFFF;
	     }

	  /* Re-enable all interrupts */
	     __enable_irq();

	  /* Set up the jump to booloader address + 4 */
	     SysMemBootJump = (void (*)(void)) (*((uint32_t *) ((BootAddr + 4))));

	  /* Set the main stack pointer to the bootloader stack */
	     __set_MSP(*(uint32_t *)BootAddr);

	  /* Call the function to jump to bootloader location */
	     SysMemBootJump();

	  /* Jump is done successfully */
	     while (1)
	     {
	      /* Code should never reach this loop */
	     }
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

/* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SAI1_Init();
  MX_SPI1_Init();
  MX_FMC_Init();
  MX_OCTOSPI1_Init();
  MX_USB_DEVICE_Init();
  /* USER CODE BEGIN 2 */

  // init SAI interface
	HAL_SAI_Transmit_DMA(&hsai_BlockA1, output_i2s_buffer_au32, 	16);
	HAL_SAI_Receive_DMA(&hsai_BlockB1, input_i2s_buffer_au32, 	16);

  // init CODEC
	ad1939_init(&hspi1);



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	// FLASH TESTING START

//	AUDIO_Init();
	JumpToBootloader();
		// FLASH TESTING END
  while (1)
  {
	  uint32_t fmctestStart;
	  uint32_t fmctestStop;

	  fmctestStart = HAL_GetTick();
	  uint32_t errorCounter =0;
	  uint32_t usb_state=0;
	  for(uint32_t i = 0; i<10000;i++){


		  //usb_state = CDC_Transmit_HS((uint8_t*)mymsg, strlen(mymsg));
		  //HAL_Delay(100);

		  for(uint32_t j=256*256-2; j<256*256+200;j++){
			  fmctestStart = HAL_GetTick();
			  uint8_t number_inc = 0;
//			  for(uint32_t counter = 0; counter<SDRAM_SIZE; counter++){
//				  *(__IO uint8_t*)(SDRAM_ADDRESS_START+6 + counter) = (uint8_t) number_inc;
//				  number_inc++;
//			  }
//			  for(uint32_t counter = 1; counter<SDRAM_SIZE; counter=counter+2){
//				  *(__IO uint8_t*)(SDRAM_ADDRESS_START + counter) = (uint8_t) number_inc;
//				  number_inc++;
//			  }

			  for(uint32_t counter = 0; counter<ARRAY_SIZE; counter++){
			 				  sdram_array[counter] = j;
			 			  }


//			  for(uint32_t counter = 0; counter<SDRAM_SIZE; counter++){
//			  				  if(*(__IO uint8_t*)(SDRAM_ADDRESS_START + counter) != j){
//			  					  errorCounter++;
//			  				  }


			  for(uint32_t counter = 0; counter<ARRAY_SIZE; counter++){
				  if( sdram_array[counter] != j){
					  errorCounter++;
				  }
			  }
			  fmctestStop = (HAL_GetTick()-fmctestStart);
		  }

	  }

	  if(errorCounter){
		  while(1){

		  }
	  }

	  uint8_t var = *(__IO uint8_t*)(SDRAM_ADDRESS_START);
	  HAL_Delay(50);

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 44;
  RCC_OscInitStruct.PLL.PLLP = 1;
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
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1|RCC_PERIPHCLK_SPI1;
  PeriphClkInitStruct.PLL3.PLL3M = 2;
  PeriphClkInitStruct.PLL3.PLL3N = 15;
  PeriphClkInitStruct.PLL3.PLL3P = 2;
  PeriphClkInitStruct.PLL3.PLL3Q = 2;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_3;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 2950;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLL3;
  PeriphClkInitStruct.Spi123ClockSelection = RCC_SPI123CLKSOURCE_PLL3;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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
