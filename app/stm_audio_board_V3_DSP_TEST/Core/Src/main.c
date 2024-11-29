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
#include <guitar_effect_octave.h>

// SAB specifics------START----
// Effects libs
#include "guitar_effect_delay.h"
#include "sab_intercom.h"

#include "FLASH_SECTOR_H7.h"
#include "SAB_custom_fx.h"
#include "SAB_fx_manager.h"

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

volatile struct{
	int32_t in1_i32;
	int32_t in2_i32;
	int32_t in3_i32;
	int32_t in4_i32;

	int32_t out1_i32;
	int32_t out2_i32;
	int32_t out3_i32;
	int32_t out4_i32;
}effects_io_port,effects_io_port_half;

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



#define SDRAM_ADDRESS_START 0xC0000000
#define SDRAM_SIZE 			0x100000 // 16Mb = 2MB

#define ARRAY_SIZE 96000
// Define a padding variable to offset the array by 2 bytes
//__attribute__((section(".sdram_section"))) volatile uint32_t sdram_array[ARRAY_SIZE];
//__attribute__((section(".sdram_section"))) volatile uint16_t sdram_byte;

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


volatile int BufSize = 4000;
volatile int Overlap = 1000;

volatile int Buf[10000];

volatile int WtrP;
volatile float Rd_P;
volatile float CrossFade;
float a0, a1, a2, b1, b2, hp_in_z1, hp_in_z2, hp_out_z1, hp_out_z2;

int Do_HighPass (int inSample) {
	//	1khz high-pass, 48k
	b1 = -1.81531792;
	b2 = 0.83098222;
	a0 = 0.91157503;
	a1 = -1.82315007;
	a2 = 0.91157503;

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
float Shift = 0.5;//1.189207115002721;
int Do_PitchShift(int sample) {

	int sum = Do_HighPass(sample);
//	 sum = sample;
	//sum up and do high-pass


	//write to ringbuffer
	Buf[WtrP] = sample;

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
delay_effects_tst delay_effect;
octave_effects_tst octave_effects_st;
SAB_custom_fx_tst custom_fx_st;

sab_intercom_tst intercom_st;

int32_t sdram_buffer_test_ai32[100]__attribute__((section(".sdram_section")));

/*
 * HW BTN INTERRUPT FUNCTIONS ----------------------------
 */

//volatile uint8_t enable_effect = 1;
volatile uint8_t preset_up_pressed = 0;
volatile uint8_t preset_down_pressed = 0;
volatile uint8_t enable_effect = 0;
// EXTI Line9 External Interrupt ISR Handler CallBackFun
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if(GPIO_Pin == FSW_BTN1_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
    {
    	HAL_GPIO_TogglePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin); // Toggle The Output (LED) Pin
    	SCB_InvalidateDCache_by_Addr((uint32_t*)&(enable_effect), sizeof(&enable_effect));
    	enable_effect = ~enable_effect;
    	SCB_CleanDCache_by_Addr((uint32_t*)&(enable_effect), sizeof(&enable_effect));
    }
    if(GPIO_Pin == FSW_BTN2_Pin) // If The INT Source Is EXTI Line9 (A9 Pin)
	{
		HAL_GPIO_TogglePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin); // Toggle The Output (LED) Pin
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


/*
 * ----------------------------
 */

float32_t log_scale(uint8_t input_value) {
    // Add 1 to avoid log(0) and calculate log(1 + input_value)
    // float log_value = log10(1 + input_value);

    // Scale to fit the 0-8 range
    float32_t scaled_value = (input_value / 256.0) *4.0f;

    return scaled_value;
}

typedef struct preset_saves_st{
	sab_loop_num_tun 	loop_data[NUM_OF_LOOPS];
	sab_fx_param_tun 	fx_params_tun[NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS][NUM_OF_MAX_PARAMS];
	sab_loopbypass_tun	loopbypass_un;
}preset_saves_tst; 


// FLASH MEMORY
//  __attribute__((section(".user_data"))) preset_saves_tst presets_flash_st[26*9];

#define USER_FLASH_ADDRESS 0x08080000  // Base address of USER_FLASH section
void save_preset_to_flash(sab_intercom_tst* self){
	preset_saves_tst preset_flash_st;
	// 1. Calculate data addr
	uint32_t preset_save_size_in_byte_u32 = 	sizeof(preset_saves_tst);
	uint32_t preset_save_size_in_word_u32 = 	preset_save_size_in_byte_u32 / 4;

	uint32_t preset_num_u32 = (self->preset_data_un.preset_Major_u8-'A')*9+self->preset_data_un.preset_Minor_u8-1;
	uint32_t save_location_addr_u32 = preset_save_size_in_byte_u32*preset_num_u32+USER_FLASH_ADDRESS;
	// COPY:
	// 2.1. loop data
	memcpy(preset_flash_st.loop_data,self->loop_data,sizeof(sab_loop_num_tun)*NUM_OF_LOOPS);
	// 2.2. fx params
	for(int i = 0; i<(NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS);i++){
		if(NULL != self->fx_param_pun[i]){
			memcpy(preset_flash_st.fx_params_tun[i],self->fx_param_pun[i],sizeof(sab_fx_param_tun)*NUM_OF_MAX_PARAMS);
		}
	}
	// 2.3. loopbypass
	memcpy(preset_flash_st.loopbypass_un.all_u8,self->loopbypass_un.all_u8,sizeof(sab_loopbypass_tun));


	// 3. save to flash
	Flash_Write_Data(save_location_addr_u32,&preset_flash_st,preset_save_size_in_word_u32);
  
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
/* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
//  SCB_EnableDCache();

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
  MX_I2C4_Init();
  /* USER CODE BEGIN 2 */

  // init SAI interface
	HAL_SAI_Transmit_DMA(&hsai_BlockA1, output_i2s_buffer_au32, 	16);
	HAL_SAI_Receive_DMA(&hsai_BlockB1, input_i2s_buffer_au32, 	16);

  // init CODEC
	ad1939_init(&hspi1);

	init_guitar_effect_delay(&delay_effect);

	SAB_octave_init(&octave_effects_st);

	SAB_custom_fx_init(&custom_fx_st);

	init_intercom(&intercom_st, 0x10,&hi2c4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	// FLASH TESTING START

//	AUDIO_Init();
//	JumpToBootloader();
		// FLASH TESTING END

	HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED3_GPIO_Port, FSW_LED3_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED4_GPIO_Port, FSW_LED4_Pin, 0);


	HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
	HAL_GPIO_WritePin(FSW_LED3_GPIO_Port, FSW_LED3_Pin, 1);
	HAL_GPIO_WritePin(FSW_LED4_GPIO_Port, FSW_LED4_Pin, 1);

	HAL_I2C_EnableListen_IT(&hi2c4);

	SCB_InvalidateDCache_by_Addr((uint32_t *)&(intercom_st.loop_data[0]), sizeof(sab_loop_num_tun));
	memcpy(&intercom_st.loop_data[0].slot2,&octave_effects_st.intercom_fx_data,sizeof(fx_data_tst));
	memcpy(&intercom_st.loop_data[0].slot1,&delay_effect.intercom_fx_data,sizeof(fx_data_tst));
	memcpy(&intercom_st.loop_data[0].slot3,&custom_fx_st.intercom_fx_data,sizeof(fx_data_tst));
	SCB_CleanDCache_by_Addr((uint32_t *)&(intercom_st.loop_data[0]), sizeof(sab_preset_num_tun));

	intercom_st.fx_param_pun[1] = octave_effects_st.intercom_parameters_aun;
	intercom_st.fx_param_pun[0] = delay_effect.intercom_parameters_aun;
	intercom_st.fx_param_pun[2] = custom_fx_st.intercom_parameters_aun;

	// Load data from flash to intercom!

	// init fx
	SAB_fx_manager_tst SAB_fx_manager_st;
//	SAB_fx_manager_init(&SAB_fx_manager_st, &intercom_st);


  while (1)
  {
	if(intercom_st.save_un.save_command>0){
		save_preset_to_flash(&intercom_st);
		intercom_st.save_un.save_command = 0;
	}

	  if(ADC_READY_FLAG){
		  ADC_READY_FLAG = 0;
		if(1 == preset_down_pressed){
			preset_down_pressed = 0;
			intercom_st.prev_preset(&intercom_st);
		}
		if(1 == preset_up_pressed){
			preset_up_pressed = 0;
			intercom_st.next_preset(&intercom_st);
		}

//		intercom_st.next_preset(&intercom_st);

	  // LOOPBACK TESTING
//	  	  effects_io_port.out1_i32 = effects_io_port.in1_i32;
	  	  effects_io_port.out2_i32 = effects_io_port.in2_i32;
	  	  effects_io_port.out3_i32 = effects_io_port.in3_i32;
	  	  effects_io_port.out4_i32 = effects_io_port.in4_i32;

	  // LOOP1
		  int32_t out;
		// TESTING
			float32_t vol_sub1 = log_scale(intercom_st.fx_param_pun[0][0].value_u8);
			float32_t vol_norm = log_scale(intercom_st.fx_param_pun[0][1].value_u8);
			float32_t vol_up1  = log_scale(intercom_st.fx_param_pun[0][2].value_u8);
		  octave_effects_st.volumes_st.clean_f32 = vol_norm;
		  octave_effects_st.volumes_st.up_1_f32 = vol_up1;
		  octave_effects_st.volumes_st.up_2_f32 = vol_up1;


		  if(enable_effect != 0){
			//   out = octave_effects_st.callback(&octave_effects_st,effects_io_port.in1_i32/2) + Do_PitchShift(effects_io_port.in1_i32/2)*vol_sub1;
			out = SAB_custom_fx_process(&custom_fx_st,effects_io_port.in1_i32,0);
//			  out = delay_effect.callback(&delay_effect,effects_io_port.in1_i32/2);
		  }else{
			  out = effects_io_port.in1_i32*1.122;
		  }

		  effects_io_port.out1_i32 = out;
//

//		  effects_io_port.out1_i32 = delay_effect.callback(&delay_effect,effects_io_port.in1_i32/2);
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
