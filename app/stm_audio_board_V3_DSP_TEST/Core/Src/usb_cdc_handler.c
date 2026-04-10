#include "usb_cdc_handler.h"
#include "usbd_cdc_if.h"  // For CDC_Transmit_HS
#include "sab_intercom.h"  // For intercom_st
#include "string.h"

// External declarations
extern USBD_HandleTypeDef hUsbDeviceHS;
extern uint8_t CDC_Transmit_HS(uint8_t* Buf, uint16_t Len);
extern sab_intercom_tst intercom_st;

// Buffer for response
uint8_t usb_cdc_response_buffer[USB_CDC_RESPONSE_SIZE];

void USB_CDC_Handler_Init(void) {
    // Initialization if needed
}

void USB_CDC_Handler_ProcessData(uint8_t* data, uint32_t len) {
    if (len == 0) return;

    uint8_t command = data[0];
    uint8_t response_len = 0;

    switch (command) {
        case CMD_GET_PRESET_NUM:
            usb_cdc_response_buffer[0] = CMD_GET_PRESET_NUM;
            usb_cdc_response_buffer[1] = intercom_st.preset_data_un.preset_Major_u8;
            usb_cdc_response_buffer[2] = intercom_st.preset_data_un.preset_Minor_u8;
            response_len = 3;
            break;

        case CMD_GET_FX_SLOTS_DATA:
            // Assume data[1] is loop number, default 0
            uint8_t loop_num = (len > 1) ? data[1] : 0;
            if (loop_num >= NUM_OF_LOOPS) loop_num = 0;
            usb_cdc_response_buffer[0] = CMD_GET_FX_SLOTS_DATA;
            usb_cdc_response_buffer[1] = loop_num;
            usb_cdc_response_buffer[2] = intercom_st.loop_data[loop_num].slot1.fx_state_en;
            usb_cdc_response_buffer[3] = intercom_st.loop_data[loop_num].slot2.fx_state_en;
            usb_cdc_response_buffer[4] = intercom_st.loop_data[loop_num].slot3.fx_state_en;
            // Add names
            memcpy(&usb_cdc_response_buffer[5], intercom_st.loop_data[loop_num].slot1.name, 10);
            memcpy(&usb_cdc_response_buffer[15], intercom_st.loop_data[loop_num].slot2.name, 10);
            memcpy(&usb_cdc_response_buffer[25], intercom_st.loop_data[loop_num].slot3.name, 10);
            response_len = 35;
            break;

        case CMD_GET_FX_SLOT_PARAMS:
            // Assume data[1] is effect slot (0-11), default 0
            uint8_t fx_slot = (len > 1) ? data[1] : 0;
            if (fx_slot >= 12) fx_slot = 0;
            
            usb_cdc_response_buffer[0] = CMD_GET_FX_SLOT_PARAMS;
            usb_cdc_response_buffer[1] = fx_slot;
            uint16_t offset = 2;
            
            // Check if the effect slot has parameters
            if (intercom_st.fx_param_pun[fx_slot] != NULL) {
                // Send all 12 parameters for this effect
                for (int param_idx = 0; param_idx < 12; param_idx++) {
                    usb_cdc_response_buffer[offset++] = intercom_st.fx_param_pun[fx_slot][param_idx].type_en;
                    usb_cdc_response_buffer[offset++] = intercom_st.fx_param_pun[fx_slot][param_idx].value_u8;
                    memcpy(&usb_cdc_response_buffer[offset], intercom_st.fx_param_pun[fx_slot][param_idx].name, 5);
                    offset += 5;
                }
            } else {
                // Effect slot has no parameters, send all unused
                for (int param_idx = 0; param_idx < 12; param_idx++) {
                    usb_cdc_response_buffer[offset++] = 0;  // PARAM_TYPE_UNUSED
                    usb_cdc_response_buffer[offset++] = 0;
                    memset(&usb_cdc_response_buffer[offset], 0, 5);
                    offset += 5;
                }
            }
            response_len = offset;
            break;

        default:
            // Unknown command, send error
            usb_cdc_response_buffer[0] = 0xFF; // Error
            response_len = 1;
            break;
    }

    if (response_len > 0) {
        USB_CDC_Handler_SendResponse(usb_cdc_response_buffer, response_len);
    }
}

void USB_CDC_Handler_SendResponse(uint8_t* data, uint32_t len) {
    CDC_Transmit_HS(data, len);
}