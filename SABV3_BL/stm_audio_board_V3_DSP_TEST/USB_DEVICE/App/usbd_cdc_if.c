/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_cdc_if.c
  * @version        : v1.0_Cube
  * @brief          : Usb device for Virtual Com Port.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "usbd_cdc_if.h"

/* USER CODE BEGIN INCLUDE */
//#include "usb_cdc_handler.h"
/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device library.
  * @{
  */

/** @addtogroup USBD_CDC_IF
  * @{
  */

/** @defgroup USBD_CDC_IF_Private_TypesDefinitions USBD_CDC_IF_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Defines USBD_CDC_IF_Private_Defines
  * @brief Private defines.
  * @{
  */

/* USER CODE BEGIN PRIVATE_DEFINES */
/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Macros USBD_CDC_IF_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_Variables USBD_CDC_IF_Private_Variables
  * @brief Private variables.
  * @{
  */

/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/** Received data over USB are stored in this buffer      */
uint8_t UserRxBufferHS[APP_RX_DATA_SIZE];

/** Data to send over USB CDC are stored in this buffer   */
uint8_t UserTxBufferHS[APP_TX_DATA_SIZE];

/* USER CODE BEGIN PRIVATE_VARIABLES */
/* SAB updater framing: magic, command, reserved, payload length, sequence,
 * payload, CRC-32.  Commands are HELLO(1), ERASE(2), WRITE(3), RESET(4).
 * WRITE payload is a little-endian offset followed by 32-byte-aligned data. */
#define SABU_MAGIC             0x55424153UL
#define SABU_APP_BASE           0x08020000UL
#define SABU_APP_END            0x080E0000UL
#define SABU_RING_SIZE          4096U
#define SABU_MAX_PAYLOAD        516U

enum SABU_COMMANDS {
  SABU_HELLO = 1,
  SABU_ERASE = 2,
  SABU_WRITE = 3,
  SABU_RESET = 4
};

static volatile uint8_t sabu_ring[SABU_RING_SIZE];
static volatile uint16_t sabu_head;
static volatile uint16_t sabu_tail;
static uint8_t sabu_frame[12U + SABU_MAX_PAYLOAD + 4U] __attribute__((aligned(32)));
static uint16_t sabu_frame_used;
static uint16_t sabu_frame_expected;
static uint8_t sabu_response[16];
static uint8_t sabu_response_pending;
static uint8_t sabu_reset_requested;

static uint32_t sabu_u32(const uint8_t *p)
{
  return ((uint32_t)p[0]) | ((uint32_t)p[1] << 8) |
         ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void sabu_put_u32(uint8_t *p, uint32_t value)
{
  p[0] = (uint8_t)value; p[1] = (uint8_t)(value >> 8);
  p[2] = (uint8_t)(value >> 16); p[3] = (uint8_t)(value >> 24);
}

static uint32_t sabu_crc32(const uint8_t *data, uint32_t length)
{
  uint32_t crc = 0xFFFFFFFFUL;
  while (length-- != 0U) {
    crc ^= *data++;
    for (uint8_t bit = 0; bit < 8U; ++bit)
      crc = (crc & 1U) ? (crc >> 1) ^ 0xEDB88320UL : (crc >> 1);
  }
  return ~crc;
}

static void sabu_reply(uint8_t command, uint8_t status, uint32_t sequence, uint32_t detail)
{
  sabu_put_u32(&sabu_response[0], SABU_MAGIC);
  sabu_response[4] = (uint8_t)(command | 0x80U);
  sabu_response[5] = status;
  sabu_response[6] = 0; sabu_response[7] = 0;
  sabu_put_u32(&sabu_response[8], sequence);
  sabu_put_u32(&sabu_response[12], detail);
  sabu_response_pending = 1U;
}

static FLASH_EraseInitTypeDef EraseInitStruct;
static uint8_t sabu_erase_application_sector(uint32_t sector)
{
    FLASH_EraseInitTypeDef erase;
    uint32_t sector_error = 0;
    HAL_StatusTypeDef status;

    if (sector > 7U)
        return 2U;

    memset(&erase, 0, sizeof(erase));

    // erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    // erase.Banks        = FLASH_BANK_1;
    // erase.Sector       = sector;
    // erase.NbSectors    = 1;
    // erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

      erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.Banks        = FLASH_BANK_1;
    erase.Sector       = 1;
    erase.NbSectors    = 4;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    __disable_irq();

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(
    FLASH_FLAG_EOP_BANK1      |
    FLASH_FLAG_OPERR_BANK1    |
    FLASH_FLAG_WRPERR_BANK1   |
    FLASH_FLAG_PGSERR_BANK1   |
    FLASH_FLAG_STRBERR_BANK1  |
    FLASH_FLAG_INCERR_BANK1   |
    FLASH_FLAG_RDPERR_BANK1   |
    FLASH_FLAG_RDSERR_BANK1   |
    FLASH_FLAG_SNECCERR_BANK1 |
    FLASH_FLAG_DBECCERR_BANK1
);

    status = HAL_FLASHEx_Erase(&erase, &sector_error);

    HAL_FLASH_Lock();

//    SCB_CleanInvalidateDCache();
//    SCB_InvalidateICache();

    __enable_irq();

    if (status != HAL_OK)
    {
        // Put a breakpoint here and inspect:
        // HAL_FLASH_GetError()
        // sector_error
        return (uint8_t)HAL_FLASH_GetError();
    }

    return 0U;
}

static uint8_t sabu_write_application(const uint8_t *payload, uint16_t length)
{
    if ((length < 36U) || (((length - 4U) & 31U) != 0U))
        return 2U;

    const uint32_t offset = sabu_u32(payload);
    uint32_t address = SABU_APP_BASE + offset;
    const uint32_t bytes = length - 4U;

    if ((address < SABU_APP_BASE) ||
        ((address + bytes) > SABU_APP_END) ||
        ((address & 31U) != 0U))
    {
        return 3U;
    }

    /* Source must be word aligned */
    if ((((uint32_t)(payload + 4U)) & 3U) != 0U)
        return 5U;

    __disable_irq();

    HAL_FLASH_Unlock();

    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS_BANK1);

    for (uint32_t i = 0; i < bytes; i += 32U)
    {
        HAL_StatusTypeDef status =
            HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD,
                              address + i,
                              (uint32_t)(payload + 4U + i));

        if (status != HAL_OK)
        {
            uint32_t err = HAL_FLASH_GetError();

            HAL_FLASH_Lock();
            // SCB_CleanInvalidateDCache();
            // SCB_InvalidateICache();
            __enable_irq();

            return (uint8_t)err;
        }
    }

    HAL_FLASH_Lock();

    // SCB_CleanInvalidateDCache();
    // SCB_InvalidateICache();

    __enable_irq();

    return 0U;
}

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Exported_Variables USBD_CDC_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceHS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_CDC_IF_Private_FunctionPrototypes USBD_CDC_IF_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t CDC_Init_HS(void);
static int8_t CDC_DeInit_HS(void);
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length);
static int8_t CDC_Receive_HS(uint8_t* pbuf, uint32_t *Len);
static int8_t CDC_TransmitCplt_HS(uint8_t *pbuf, uint32_t *Len, uint8_t epnum);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_CDC_ItfTypeDef USBD_Interface_fops_HS =
{
  CDC_Init_HS,
  CDC_DeInit_HS,
  CDC_Control_HS,
  CDC_Receive_HS,
  CDC_TransmitCplt_HS
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes the CDC media low layer over the USB HS IP
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Init_HS(void)
{
  /* USER CODE BEGIN 8 */
  /* Set Application Buffers */
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, UserTxBufferHS, 0);
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, UserRxBufferHS);
  return (USBD_OK);
  /* USER CODE END 8 */
}

/**
  * @brief  DeInitializes the CDC media low layer
  * @param  None
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_DeInit_HS(void)
{
  /* USER CODE BEGIN 9 */
  return (USBD_OK);
  /* USER CODE END 9 */
}

/**
  * @brief  Manage the CDC class requests
  * @param  cmd: Command code
  * @param  pbuf: Buffer containing command data (request parameters)
  * @param  length: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_Control_HS(uint8_t cmd, uint8_t* pbuf, uint16_t length)
{
  /* USER CODE BEGIN 10 */
  switch(cmd)
  {
  case CDC_SEND_ENCAPSULATED_COMMAND:

    break;

  case CDC_GET_ENCAPSULATED_RESPONSE:

    break;

  case CDC_SET_COMM_FEATURE:

    break;

  case CDC_GET_COMM_FEATURE:

    break;

  case CDC_CLEAR_COMM_FEATURE:

    break;

  /*******************************************************************************/
  /* Line Coding Structure                                                       */
  /*-----------------------------------------------------------------------------*/
  /* Offset | Field       | Size | Value  | Description                          */
  /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
  /* 4      | bCharFormat |   1  | Number | Stop bits                            */
  /*                                        0 - 1 Stop bit                       */
  /*                                        1 - 1.5 Stop bits                    */
  /*                                        2 - 2 Stop bits                      */
  /* 5      | bParityType |  1   | Number | Parity                               */
  /*                                        0 - None                             */
  /*                                        1 - Odd                              */
  /*                                        2 - Even                             */
  /*                                        3 - Mark                             */
  /*                                        4 - Space                            */
  /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
  /*******************************************************************************/
  case CDC_SET_LINE_CODING:

    break;

  case CDC_GET_LINE_CODING:

    break;

  case CDC_SET_CONTROL_LINE_STATE:

    break;

  case CDC_SEND_BREAK:

    break;

  default:
    break;
  }

  return (USBD_OK);
  /* USER CODE END 10 */
}

/**
  * @brief Data received over USB OUT endpoint are sent over CDC interface
  *         through this function.
  *
  *         @note
  *         This function will issue a NAK packet on any OUT packet received on
  *         USB endpoint until exiting this function. If you exit this function
  *         before transfer is complete on CDC interface (ie. using DMA controller)
  *         it will result in receiving more data while previous ones are still
  *         not sent.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAILL
  */
static int8_t CDC_Receive_HS(uint8_t* Buf, uint32_t *Len)
{
  /* USER CODE BEGIN 11 */
  /* Only enqueue here: flash erase/programming must not run in the USB IRQ. */
  for (uint32_t i = 0; i < *Len; ++i) {
    const uint16_t next = (uint16_t)((sabu_head + 1U) % SABU_RING_SIZE);
    if (next == sabu_tail) {
      /* Drop a damaged stream; the host will time out and report the error. */
      sabu_tail = sabu_head;
      break;
    }
    sabu_ring[sabu_head] = Buf[i];
    sabu_head = next;
  }

  // Prepare for next reception
  USBD_CDC_SetRxBuffer(&hUsbDeviceHS, &Buf[0]);
  USBD_CDC_ReceivePacket(&hUsbDeviceHS);
  return (USBD_OK);
  /* USER CODE END 11 */
}

/**
  * @brief  Data to send over USB IN endpoint are sent over CDC interface
  *         through this function.
  * @param  Buf: Buffer of data to be sent
  * @param  Len: Number of data to be sent (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
  */
uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 12 */
  USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)hUsbDeviceHS.pClassData;
  if (hcdc->TxState != 0){
    return USBD_BUSY;
  }
  USBD_CDC_SetTxBuffer(&hUsbDeviceHS, Buf, Len);
  result = USBD_CDC_TransmitPacket(&hUsbDeviceHS);
  /* USER CODE END 12 */
  return result;
}

/**
  * @brief  CDC_TransmitCplt_HS
  *         Data transmitted callback
  *
  *         @note
  *         This function is IN transfer complete callback used to inform user that
  *         the submitted Data is successfully sent over USB.
  *
  * @param  Buf: Buffer of data to be received
  * @param  Len: Number of data received (in bytes)
  * @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
  */
static int8_t CDC_TransmitCplt_HS(uint8_t *Buf, uint32_t *Len, uint8_t epnum)
{
  uint8_t result = USBD_OK;
  /* USER CODE BEGIN 14 */
  UNUSED(Buf);
  UNUSED(Len);
  UNUSED(epnum);
  /* USER CODE END 14 */
  return result;
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

void FW_Update_Process(void)
{
  if (sabu_response_pending != 0U) {
    if (CDC_Transmit_HS(sabu_response, sizeof(sabu_response)) == USBD_OK)
      sabu_response_pending = 0U;
    return;
  }

  while (sabu_tail != sabu_head) {
    const uint8_t byte = sabu_ring[sabu_tail];
    sabu_tail = (uint16_t)((sabu_tail + 1U) % SABU_RING_SIZE);

    if (sabu_frame_used < sizeof(sabu_frame))
      sabu_frame[sabu_frame_used++] = byte;
    else
      sabu_frame_used = 0U;

    if (sabu_frame_used == 12U) {
      const uint16_t payload_length = (uint16_t)sabu_frame[6] |
                                      ((uint16_t)sabu_frame[7] << 8);
      if ((sabu_u32(sabu_frame) != SABU_MAGIC) ||
          (payload_length > SABU_MAX_PAYLOAD)) {
        sabu_frame_used = 0U;
        continue;
      }
      sabu_frame_expected = (uint16_t)(12U + payload_length + 4U);
    }

    if ((sabu_frame_expected != 0U) && (sabu_frame_used == sabu_frame_expected)) {
      const uint8_t command = sabu_frame[4];
      const uint16_t length = (uint16_t)sabu_frame[6] | ((uint16_t)sabu_frame[7] << 8);
      const uint32_t sequence = sabu_u32(&sabu_frame[8]);
      const uint32_t received_crc = sabu_u32(&sabu_frame[12U + length]);
      uint8_t status = 0U;
      uint32_t detail = 0U;

      if (received_crc != sabu_crc32(sabu_frame, 12U + length)) {
        status = 10U;
      } else if (command == SABU_HELLO) {
        detail = SABU_APP_END - SABU_APP_BASE;
      } else if (command == SABU_ERASE) {
        if (length != 1U) {
          status = 12U;
        } else {
          detail = sabu_frame[12];
          status = sabu_erase_application_sector(sabu_frame[12]);
        }
      } else if (command == SABU_WRITE) {
        status = sabu_write_application(&sabu_frame[12], length);
      } else if (command == SABU_RESET) {
          sabu_reset_requested = 1U;
      } else {
        status = 11U;
      }

      sabu_reply(command, status, sequence, detail);
      sabu_frame_used = 0U;
      sabu_frame_expected = 0U;
      return;
    }
  }
}

uint8_t FW_Update_ResetRequested(void)
{
  return sabu_reset_requested;
}

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */
