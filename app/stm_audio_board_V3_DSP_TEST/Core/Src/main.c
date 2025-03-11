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
#include "i2c.h"
#include "octospi.h"
#include "sai.h"
#include "spi.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32h7xx_hal.h"
#include "AD1939_driver.h"
// #include "usbd_core.h"
//#include "usbd_audio.h"
// #include "usbd_desc.h"
//#include "usbd_audio_if.h"
// #include "usb_device.h"
#include "string.h"
#include <math.h>
#include "arm_math.h"

// SAB specifics------START----
// Effects libs
#include "sab_intercom.h"

#include "FLASH_SECTOR_H7.h"
#include "SAB_custom_fx.h"
#include "SAB_fx_manager.h"
#include "sdram_memory_handler.h"

// SAB specifics------END----
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

// IN2 - OUT1
//

// ADC4-ADC1 position in TDM I2S
#define IN1_ADC_NUM 3
#define IN2_ADC_NUM 2
#define IN3_ADC_NUM 1
#define IN4_ADC_NUM 0
// DAC position in TDM I2S
#define OUT1_DAC_NUM 7 // ok
#define OUT2_DAC_NUM 6
#define OUT3_DAC_NUM 4
#define OUT4_DAC_NUM 5

volatile uint8_t 			ADC_HALF_COMPLETE_FLAG = 0;
volatile uint8_t 			DAC_HALF_COMPLETE_FLAG = 0;
uint32_t input_i2s_buffer_au32[16];
uint32_t output_i2s_buffer_au32[16];

SAB_IO_HADRWARE_BUFFERS effects_io_port;

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 0;
	output_i2s_buffer_au32[8+OUT1_DAC_NUM] = effects_io_port.out1_i32>>8;
	output_i2s_buffer_au32[8+OUT2_DAC_NUM] = effects_io_port.out2_i32>>8;
	output_i2s_buffer_au32[8+OUT3_DAC_NUM] = effects_io_port.out3_i32>>8;
	output_i2s_buffer_au32[8+OUT4_DAC_NUM] = effects_io_port.out4_i32>>8;
	SCB_CleanDCache_by_Addr(&output_i2s_buffer_au32[8], sizeof(output_i2s_buffer_au32)/2);
}
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 1;
	output_i2s_buffer_au32[OUT1_DAC_NUM] = effects_io_port.out1_i32>>8;
	output_i2s_buffer_au32[OUT2_DAC_NUM] = effects_io_port.out2_i32>>8;
	output_i2s_buffer_au32[OUT3_DAC_NUM] = effects_io_port.out3_i32>>8;
	output_i2s_buffer_au32[OUT4_DAC_NUM] = effects_io_port.out4_i32>>8;
	SCB_CleanDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32)/2);
}

volatile uint8_t 			ADC_READY_FLAG = 0;

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 0;
	ADC_READY_FLAG = 1;
	SCB_InvalidateDCache_by_Addr(&input_i2s_buffer_au32[8], sizeof(input_i2s_buffer_au32)/2);
	effects_io_port.in1_i32 = input_i2s_buffer_au32[8+IN1_ADC_NUM]<<8;
	effects_io_port.in2_i32 = input_i2s_buffer_au32[8+IN2_ADC_NUM]<<8;
	effects_io_port.in3_i32 = input_i2s_buffer_au32[8+IN3_ADC_NUM]<<8;
	effects_io_port.in4_i32 = input_i2s_buffer_au32[8+IN4_ADC_NUM]<<8;
}
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 1;
	ADC_READY_FLAG = 1;
	SCB_InvalidateDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32)/2);
	effects_io_port.in1_i32 = input_i2s_buffer_au32[IN1_ADC_NUM]<<8;
	effects_io_port.in2_i32 = input_i2s_buffer_au32[IN2_ADC_NUM]<<8;
	effects_io_port.in3_i32 = input_i2s_buffer_au32[IN3_ADC_NUM]<<8;
	effects_io_port.in4_i32 = input_i2s_buffer_au32[IN4_ADC_NUM]<<8;
}

#define ARRAY_SIZE 96000

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


sab_intercom_tst 	 intercom_st __attribute__((section(".noncacheable_data")));
SAB_fx_manager_tst SAB_fx_manager_st;

/*
 * HW BTN INTERRUPT FUNCTIONS ----------------------------
 */

//volatile uint8_t enable_effect = 1;
volatile uint8_t preset_up_pressed = 0;
volatile uint8_t preset_down_pressed = 0;
volatile uint8_t fsw_btn_1_pressed __attribute__((section(".noncacheable_data"))) = 0;
volatile uint8_t fsw_btn_2_pressed __attribute__((section(".noncacheable_data"))) = 0;
// EXTI Line9 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == FSW_BTN1_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
    {
    	fsw_btn_1_pressed = 1;
    }
    if(GPIO_Pin == FSW_BTN2_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
	{
		fsw_btn_2_pressed = 1;
	}
    if(GPIO_Pin == FSW_BTN3_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
	{
		HAL_GPIO_TogglePin(FSW_LED3_GPIO_Port, FSW_LED3_Pin); // Toggle The Output (LED) Pin
		preset_up_pressed = 1;
	}
    if(GPIO_Pin == FSW_BTN4_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
	{
		HAL_GPIO_TogglePin(FSW_LED4_GPIO_Port, FSW_LED4_Pin); // Toggle The Output (LED) Pin
		preset_down_pressed = 1;
	}
}


/*
 * I2C INTERRUPT FUNCTIONS ----------------------------
 */


#define RX_LEN 255
#define TX_LEN 255
uint8_t TX_Buffer [TX_LEN] = "ABCDEF" ; // DATA to send
volatile uint8_t RX_Buffer [RX_LEN] = "ABCDEF" ; // DATA to send
uint8_t tx_counter = 0;
uint8_t rx_counter = 0;
uint8_t commando = 0;
volatile uint8_t rx_reg = 0;

extern void HAL_I2C_ListenCpltCallback (I2C_HandleTypeDef *hi2c)
{
	HAL_I2C_EnableListen_IT(hi2c);
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef *hi2c)
{
	if(rx_counter>0){
		intercom_st.register_addr_u8 = rx_reg;
		intercom_st.process_rx_buffer(&intercom_st, RX_Buffer, rx_counter-1);
		rx_counter = 0;
	}
	HAL_I2C_EnableListen_IT(hi2c);
}

extern void HAL_I2C_AddrCallback(I2C_HandleTypeDef *hi2c, uint8_t TransferDirection, uint16_t AddrMatchCode)
{
	if(TransferDirection == I2C_DIRECTION_TRANSMIT)  
	{
		HAL_I2C_Slave_Sequential_Receive_IT(hi2c, &intercom_st.register_addr_buffer_u8, 1, I2C_FIRST_FRAME);
		rx_counter = 0;
	}
	else
	{
		intercom_st.register_addr_u8 = rx_reg;
		// if the master wants to READ the data
		HAL_I2C_Slave_Seq_Transmit_IT(hi2c, intercom_st.get_reg_data_ptr(&intercom_st), intercom_st.get_reg_data_len(&intercom_st), I2C_FIRST_AND_LAST_FRAME);
	}
}

void HAL_I2C_SlaveRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(0 == rx_counter){
		rx_reg = intercom_st.register_addr_buffer_u8;
	}
	HAL_I2C_Slave_Sequential_Receive_IT(hi2c, RX_Buffer+rx_counter, 1, I2C_NEXT_FRAME);
	rx_counter++;
}


void HAL_I2C_SlaveTxCpltCallback(I2C_HandleTypeDef *hi2c)
{
}

void I2C_Slave_Listen(void) {
	HAL_I2C_Slave_Receive_IT(&hi2c4, RX_Buffer, sizeof(RX_Buffer));
}

// Variables to store cycle counts
uint32_t startCycles, endCycles, totalCycles;
float32_t time_to_process_f32;
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
/* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_I2C4_Init();
  /* USER CODE BEGIN 2 */

  // init SAI interface
	HAL_SAI_Transmit_DMA(&hsai_BlockA1, output_i2s_buffer_au32, 	16);
	HAL_SAI_Receive_DMA(&hsai_BlockB1, input_i2s_buffer_au32, 	16);

  // init CODEC
	ad1939_init(&hspi1);
	init_intercom(&intercom_st, 0x10,&hi2c4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED3_GPIO_Port, FSW_LED3_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED4_GPIO_Port, FSW_LED4_Pin, 0);


	HAL_GPIO_WritePin(FSW_LED3_GPIO_Port, FSW_LED3_Pin, 1);
	HAL_GPIO_WritePin(FSW_LED4_GPIO_Port, FSW_LED4_Pin, 1);

	HAL_I2C_EnableListen_IT(&hi2c4);

	SAB_fx_manager_init(&SAB_fx_manager_st, &intercom_st, &effects_io_port, &fsw_btn_1_pressed, &fsw_btn_2_pressed);
	
    /* Enable the TRC (Trace) */
    // CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    // /* Unlock the DWT if it is locked (LAR register) */
    // // Some Cortex-M7 MCUs (including STM32H7) require this unlock sequence:
    // DWT->LAR = 0xC5ACCE55;

    // /* Reset the cycle counter */
    // DWT->CYCCNT = 0;

    // /* Enable the cycle counter */
    // DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
    clear_sdram_memory();
  while (1)
  {
	if(intercom_st.save_un.save_command>0){
		SAB_save_preset_to_flash(&SAB_fx_manager_st);
		intercom_st.save_un.save_command = 0;
	}
	if(intercom_st.dsp_fw_update_flg){
		JumpToBootloader();
	}

	  if(ADC_READY_FLAG){
		  ADC_READY_FLAG = 0;
		  // // Reset the counter to ensure a clean start
		  //     DWT->CYCCNT = 0;

		  //     // Record the starting cycle count
		  //     startCycles = DWT->CYCCNT;
		SAB_fx_manager_process(&SAB_fx_manager_st);
		 // Record the ending cycle count
		    // endCycles = DWT->CYCCNT;

		    // // Calculate the difference
		    // totalCycles = endCycles - startCycles;

		    // time_to_process_f32 = (float)totalCycles/550;
		if(1 == preset_down_pressed){
			preset_down_pressed = 0;
			SAB_preset_down_pressed(&SAB_fx_manager_st);
		}
		if(1 == preset_up_pressed){
			preset_up_pressed = 0;
			SAB_preset_up_pressed(&SAB_fx_manager_st);
		}



		if(fsw_btn_1_pressed | fsw_btn_2_pressed){
			SAB_fsw_pressed_callback(&SAB_fx_manager_st);
			switch (SAB_fx_manager_st.preset_mode_st.preset_mode_en)
			{
			case PRESET_MODE_A_ACTIVE:
				HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 1);
				HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
				break;
			case PRESET_MODE_B_ACTIVE:
				HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
				HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 1);
				break;
			
			default:
				HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
				HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
				break;
			}
			fsw_btn_1_pressed = 0;
			fsw_btn_2_pressed = 0;
		}



	  }
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
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
	HAL_GPIO_WritePin(GPIOG, GPIO_PIN_5, GPIO_PIN_RESET);
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
