/*
 * sab_intercom.c
 *
 *  Created on: Oct 11, 2024
 *      Author: pappr
 */

#include "sab_intercom.h"

// ----------------------------------GETTERS-------------------------------------------------
static void get_preset_data (struct sab_intercom_st* self){
    HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
                        SAB_I2C_REG_PRESETNUM, I2C_MEMADD_SIZE_8BIT,
                        &self->preset_data_un.all_u32, SAB_I2C_REG_PRESETNUM_LEN,
                        1000);
}
static void get_loop_data (struct sab_intercom_st* self, uint8_t loop_num_u8){
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
						SAB_I2C_REG_LOOP1FX+loop_num_u8-1,
						I2C_MEMADD_SIZE_8BIT,
						self->loop_data->all_pau8, SAB_I2C_REG_LOOPFX_LEN,
						1000);
}
static void get_fx_param(struct sab_intercom_st* self, uint8_t param_slot_u8){
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
						SAB_I2C_REG_FXPARAM1+param_slot_u8-1,
						I2C_MEMADD_SIZE_8BIT,
						&self->fx_param_un[param_slot_u8-1].all_au8, SAB_I2C_REG_FXPARAM_LEN,
						1000);
}

static void get_info (struct sab_intercom_st* self){
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
						SAB_I2C_REG_INFO,
						I2C_MEMADD_SIZE_8BIT,
						&self->info_un.all_u8, SAB_I2C_REG_INFO_LEN,
						1000);
}
static void get_loopbypass (struct sab_intercom_st* self){
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
						SAB_I2C_REG_LOOPBYPASSSTATE,
						I2C_MEMADD_SIZE_8BIT,
						&self->loopbypass_un.all_u8, SAB_I2C_REG_LOOPBYPASSSTATE_LEN,
						1000);
}

// ----------------------------------SETTERS-------------------------------------------------
static void set_fx_param (struct sab_intercom_st* self, uint8_t param_slot_u8, uint8_t new_value_u8){
	self->fx_param_un[param_slot_u8-1].value_u8 = new_value_u8;

	HAL_I2C_Mem_Write(self->i2c_h,self->slave_addr_u8,
			SAB_I2C_REG_FXPARAM1+param_slot_u8-1,
			I2C_MEMADD_SIZE_8BIT,
			&self->fx_param_un[param_slot_u8-1], SAB_I2C_REG_FXPARAM_LEN, 1000);
}

static void set_loopbypass (struct sab_intercom_st* self){
	HAL_I2C_Mem_Write(self->i2c_h,self->slave_addr_u8,
				SAB_I2C_REG_LOOPBYPASSSTATE,
				I2C_MEMADD_SIZE_8BIT,
				&self->loopbypass_un.all_u8, SAB_I2C_REG_LOOPBYPASSSTATE_LEN, 1000);
}

// ----------------------------------INIT-------------------------------------------------
void init_intercom(struct sab_intercom_st* self, uint8_t slave_address_u8,I2C_HandleTypeDef *i2c_h){
    self->slave_addr_u8 = slave_address_u8<<1;
    self->i2c_h = i2c_h;
    // GETTER function pointers
    self->get_preset_data   = get_preset_data;
    self->get_loop_data     = get_loop_data  ;
    self->get_fx_param		= get_fx_param   ;
    self->get_info          = get_info       ;
    self->get_loopbypass    = get_loopbypass ;

    // SETTER function pointers
    self->set_fx_param      = set_fx_param;
    self->set_loopbypass	= set_loopbypass;
}

static test_fx_params_fill(struct sab_intercom_st* self){
	strcpy(self->fx_param_un[0].name, "TIME");  // Delay time parameter
		self->fx_param_un[0].type_en = 5;  // Type: Delay
		self->fx_param_un[0].value_u8 = 69;  // Value in ms

		strcpy(self->fx_param_un[1].name, "RATE");  // Modulation rate
		self->fx_param_un[1].type_en = 2;  // Type: Modulation
		self->fx_param_un[1].value_u8 = 120;  // Value in Hz

		strcpy(self->fx_param_un[2].name, "DEPTH");  // Effect depth
		self->fx_param_un[2].type_en = 3;  // Type: Modulation Depth
		self->fx_param_un[2].value_u8 = 85;  // Percentage

		strcpy(self->fx_param_un[3].name, "GAIN");  // Amplification gain
		self->fx_param_un[3].type_en = 1;  // Type: Amplification
		self->fx_param_un[3].value_u8 = 100;  // Value in dB

		strcpy(self->fx_param_un[4].name, "MIX");  // Dry/Wet mix
		self->fx_param_un[4].type_en = 4;  // Type: Mix
		self->fx_param_un[4].value_u8 = 50;  // 50% mix

		strcpy(self->fx_param_un[5].name, "DECAY");  // Reverb decay
		self->fx_param_un[5].type_en = 6;  // Type: Reverb
		self->fx_param_un[5].value_u8 = 72;  // Value in percentage

		strcpy(self->fx_param_un[6].name, "FREQ");  // Filter cutoff frequency
		self->fx_param_un[6].type_en = 7;  // Type: Filter
		self->fx_param_un[6].value_u8 = 200;  // Frequency in Hz

		strcpy(self->fx_param_un[7].name, "RES");  // Filter resonance
		self->fx_param_un[7].type_en = 7;  // Type: Filter
		self->fx_param_un[7].value_u8 = 60;  // Resonance value

		strcpy(self->fx_param_un[8].name, "LEVEL");  // Output level
		self->fx_param_un[8].type_en = 1;  // Type: Amplification
		self->fx_param_un[8].value_u8 = 95;  // Value in dB

		strcpy(self->fx_param_un[9].name, "FEED");  // Feedback amount
		self->fx_param_un[9].type_en = 5;  // Type: Delay/Feedback
		self->fx_param_un[9].value_u8 = 40;  // Percentage

		strcpy(self->fx_param_un[10].name, "THRSH");  // Compression threshold
		self->fx_param_un[10].type_en = 8;  // Type: Compression
		self->fx_param_un[10].value_u8 = 128;  // Threshold level

		strcpy(self->fx_param_un[11].name, "SPD");  // Tremolo speed
		self->fx_param_un[11].type_en = 2;  // Type: Tremolo
		self->fx_param_un[11].value_u8 = 90;  // Speed in Hz
}
void testing_data(struct sab_intercom_st* self){
	test_fx_params_fill(self);


}
