#ifndef INC_SAB_INTERCOM_H
#define INC_SAB_INTERCOM_H


#include "sab_i2c_protocol_types.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_i2c.h"
#include "string.h"
#include "stdint.h"
#include <stdlib.h>
// THis file contains the communication basics for the I2C protocoll
// between the DSP and DISPLAY unit
#define NUM_OF_LOOPS 4

typedef struct sab_intercom_st{
    uint8_t slave_addr_u8;
    uint8_t register_addr_u8;
    uint8_t register_addr_buffer_u8;
    I2C_HandleTypeDef *i2c_h;
    // data types
    sab_preset_num_tun  preset_data_un;
    sab_loop_num_tun    loop_data[NUM_OF_LOOPS];
    sab_fx_param_tun    fx_param_un[12];
    sab_info_tun        info_un;
    sab_save_tun        save_un;
    sab_loopbypass_tun  loopbypass_un;
    
    fx_data_tst*    implemented_fx_data_ptr;
    uint8_t         num_of_implemented_effects;

    // GETTER (read)
    void (*get_preset_data) (struct sab_intercom_st* self);
    void (*get_loop_data)   (struct sab_intercom_st* self, uint8_t loop_num_u8);
    void (*get_fx_param)    (struct sab_intercom_st* self, uint8_t param_slot_u8);
    void (*get_info)        (struct sab_intercom_st* self);
    void (*get_loopbypass)  (struct sab_intercom_st* self);
    void (*get_implemented_effects)  (struct sab_intercom_st* self);

    // SETTER (write)
    void (*set_fx_param)     (struct sab_intercom_st* self, uint8_t param_slot_u8, uint8_t new_value_u8);
    void (*set_loopbypass)  (struct sab_intercom_st* self);
    void (*set_loop_data)  (struct sab_intercom_st* self,sab_loop_num_tun* loop_data_pun, uint8_t loop_num);

    void (*next_preset)(struct sab_intercom_st* self);
    void (*prev_preset)(struct sab_intercom_st* self);

    // I2C comm handlers
    void (*process_rx_buffer) (struct sab_intercom_st* self, uint8_t*buffer_pu8, uint8_t size_u8);
    uint32_t (*get_reg_data_ptr)  (struct sab_intercom_st* self);
    uint8_t (*get_reg_data_len)  (struct sab_intercom_st* self);
}sab_intercom_tst;

void init_intercom(struct sab_intercom_st* self,uint8_t slave_address_u8, I2C_HandleTypeDef *i2c_h);
void testing_data(struct sab_intercom_st* self);
void sab_intercom_rx_callback  (struct sab_intercom_st* self);
#endif /* INC_SAB_I2C_INTERCOM_H_ */
