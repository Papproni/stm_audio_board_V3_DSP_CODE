#ifndef USB_CDC_HANDLER_H
#define USB_CDC_HANDLER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "main.h"

// Command definitions
#define CMD_GET_PRESET_NUM     0x01
#define CMD_GET_FX_SLOTS_DATA  0x02
#define CMD_GET_FX_SLOT_PARAMS 0x03

// Response buffer size
#define USB_CDC_RESPONSE_SIZE 1024

// Function prototypes
void USB_CDC_Handler_Init(void);
void USB_CDC_Handler_ProcessData(uint8_t* data, uint32_t len);
void USB_CDC_Handler_SendResponse(uint8_t* data, uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* USB_CDC_HANDLER_H */