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
#include "arm_math.h"

// Effects libs
#include "guitar_effect_delay.h"
#include "guitar_effect_octave.h"

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

// ADC4-ADC1 position in TDM I2S
#define IN1_ADC_NUM 3
#define IN2_ADC_NUM 2
#define IN3_ADC_NUM 1
#define IN4_ADC_NUM 0
// DAC position in TDM I2S
#define OUT1_DAC_NUM 5
#define OUT2_DAC_NUM 7
#define OUT3_DAC_NUM 6
#define OUT4_DAC_NUM 4

volatile uint8_t 			ADC_HALF_COMPLETE_FLAG = 0;
volatile uint8_t 			DAC_HALF_COMPLETE_FLAG = 0;
volatile uint32_t input_i2s_buffer_au32[16];
volatile uint32_t output_i2s_buffer_au32[16];

struct{
	int32_t in1_i32;
	int32_t in2_i32;
	int32_t in3_i32;
	int32_t in4_i32;

	int32_t out1_i32;
	int32_t out2_i32;
	int32_t out3_i32;
	int32_t out4_i32;
}effects_io_port;

void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 0;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));

	output_i2s_buffer_au32[8+OUT1_DAC_NUM] = effects_io_port.out1_i32;
	output_i2s_buffer_au32[8+OUT2_DAC_NUM] = effects_io_port.out2_i32;
	output_i2s_buffer_au32[8+OUT3_DAC_NUM] = effects_io_port.out3_i32;
	output_i2s_buffer_au32[8+OUT4_DAC_NUM] = effects_io_port.out4_i32;
}
void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	DAC_HALF_COMPLETE_FLAG = 1;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));

	output_i2s_buffer_au32[OUT1_DAC_NUM] = effects_io_port.out1_i32;
	output_i2s_buffer_au32[OUT2_DAC_NUM] = effects_io_port.out2_i32;
	output_i2s_buffer_au32[OUT3_DAC_NUM] = effects_io_port.out3_i32;
	output_i2s_buffer_au32[OUT4_DAC_NUM] = effects_io_port.out4_i32;
}

volatile uint8_t 			ADC_READY_FLAG = 0;

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 0;
	ADC_READY_FLAG = 1;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));

	effects_io_port.in1_i32 = input_i2s_buffer_au32[8+IN1_ADC_NUM];
	effects_io_port.in2_i32 = input_i2s_buffer_au32[8+IN2_ADC_NUM];
	effects_io_port.in3_i32 = input_i2s_buffer_au32[8+IN3_ADC_NUM];
	effects_io_port.in4_i32 = input_i2s_buffer_au32[8+IN4_ADC_NUM];

	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
}
void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai){
	ADC_HALF_COMPLETE_FLAG = 1;
	ADC_READY_FLAG = 1;
	SCB_CleanDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));

	effects_io_port.in1_i32 = input_i2s_buffer_au32[IN1_ADC_NUM];
	effects_io_port.in2_i32 = input_i2s_buffer_au32[IN2_ADC_NUM];
	effects_io_port.in3_i32 = input_i2s_buffer_au32[IN3_ADC_NUM];
	effects_io_port.in4_i32 = input_i2s_buffer_au32[IN4_ADC_NUM];

	SCB_CleanDCache_by_Addr(output_i2s_buffer_au32, sizeof(output_i2s_buffer_au32));
	SCB_InvalidateDCache_by_Addr(input_i2s_buffer_au32, sizeof(input_i2s_buffer_au32));
}



#define SDRAM_ADDRESS_START 0xC0000000
#define SDRAM_SIZE 			0x100000 // 16Mb = 2MB

#define ARRAY_SIZE 96000
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


volatile int BufSize = 5000;
volatile int Overlap = 500;

volatile int Buf[10000];

volatile int WtrP;
volatile float Rd_P;
volatile float CrossFade;
float a0, a1, a2, b1, b2, hp_in_z1, hp_in_z2, hp_out_z1, hp_out_z2;

int Do_HighPass (int inSample) {
	//	500hz high-pass, 48k
		a0 = 0.9547676565107223;
		a1 = -1.9095353130214445;
		a2 = 0.9547676565107223;
		b1 =-1.9074888914066748;
		b2 =0.9115817346362142;
////	3khz high-pass, 96k
//	a0 = 0.9907853255903974;
//	a1 = -1.981570651180795;
//	a2 = 0.9907853255903974;
//	b1 = -1.9814857645620922;
//	b2 = 0.9816555377994975;
//
//	a0 = 0.3009556244638557;
//	a1 = 0;
//	a2 = -0.3009556244638557;
//	b1 = -1.1091783806868014;
//	b2 = 0.39808875107228864;

	// 10khz LOWPASS
//	a0 = 0.22018120452501466;
//	a1 = 0.4403624090500293;
//	a2 = 0.22018120452501466;
//	b1 = -0.3075475090293954;
//	b2 = 0.18827232712945405;


	float inSampleF = (float)inSample;
	float outSampleF =
			a0 * inSampleF
			+ a1 * hp_in_z1
			+ a2 * hp_in_z2
			- b1 * hp_out_z1
			- b2 * hp_out_z2;
	hp_in_z2 = hp_in_z1;
	hp_in_z1 = inSampleF;
	hp_out_z2 = hp_out_z1;
	hp_out_z1 = outSampleF;

	return (int) outSampleF;
}
float Shift = 1.5;//1.189207115002721;
int Do_PitchShift(int sample) {

	int sum = Do_HighPass(sample);
//	 sum = sample;
	//sum up and do high-pass


	//write to ringbuffer
	Buf[WtrP] = sum;

	//read fractional readpointer and generate 0° and 180° read-pointer in integer
	int RdPtr_Int = roundf(Rd_P);
	int RdPtr_Int2 = 0;
	if (RdPtr_Int >= BufSize/2) RdPtr_Int2 = RdPtr_Int - (BufSize/2);
	else RdPtr_Int2 = RdPtr_Int + (BufSize/2);

	//read the two samples...
	float Rd0 = (float) Buf[RdPtr_Int];
	float Rd1 = (float) Buf[RdPtr_Int2];

	//Check if first readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to second read-pointer
	if (Overlap >= (WtrP-RdPtr_Int) && (WtrP-RdPtr_Int) >= 0 && Shift!=1.0f) {
		int rel = WtrP-RdPtr_Int;
		CrossFade = ((float)rel)/(float)Overlap;
	}
	else if (WtrP-RdPtr_Int == 0) CrossFade = 0.0f;

	//Check if second readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to first read-pointer
	if (Overlap >= (WtrP-RdPtr_Int2) && (WtrP-RdPtr_Int2) >= 0 && Shift!=1.0f) {
			int rel = WtrP-RdPtr_Int2;
			CrossFade = 1.0f - ((float)rel)/(float)Overlap;
		}
	else if (WtrP-RdPtr_Int2 == 0) CrossFade = 1.0f;


	//do cross-fading and sum up
	sum = (Rd0*CrossFade + Rd1*(1.0f-CrossFade));

	//increment fractional read-pointer and write-pointer
	Rd_P += Shift;
	WtrP++;
	if (WtrP == BufSize) WtrP = 0;
	if (roundf(Rd_P) >= BufSize) Rd_P = 0.0f;

	return sum;
}

// Effect instances
__attribute__((section(".sdram_section"))) delay_effects_tst delay_effect;
octave_effects_tst octave_effects_st;
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

	init_guitar_effect_delay(&delay_effect);

	init_guitar_effect_octave(&octave_effects_st);


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	// FLASH TESTING START

//	AUDIO_Init();
//	JumpToBootloader();
		// FLASH TESTING END
  while (1)
  {
	  // LOOP1
	  effects_io_port.out1_i32 = octave_effects_st.callback(&octave_effects_st,effects_io_port.in1_i32);
	  effects_io_port.out1_i32= Do_PitchShift(effects_io_port.in1_i32) + effects_io_port.out1_i32;

	  // LOOP2
	  effects_io_port.out2_i32 = delay_effect.callback(&delay_effect,effects_io_port.in2_i32);

	  // LOOP3

	  // LOOP4



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
