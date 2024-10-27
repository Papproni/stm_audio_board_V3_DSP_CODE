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

/// @brief Saves data received from master (display)
/// @param self 
/// @param buffer_pu8 
/// @param size_u8 
void sab_intercom_process_i2c_data(struct sab_intercom_st* self, uint8_t*buffer_pu8, uint8_t size_u8){
	memcpy(self->get_reg_data_ptr(self),buffer_pu8,size_u8);
}

/// @brief This function returns the data pointer which is adressed by the "register_addr_u8"
/// @param self 
/// @return 
uint32_t sab_intercom_get_reg_data_ptr(struct sab_intercom_st* self){
	switch (self->register_addr_u8)
	{
		case SAB_I2C_REG_PRESETNUM:
			return &self->preset_data_un;  // Returns pointer to the preset data union

		case SAB_I2C_REG_LOOP1FX:
			return &self->loop_data[0];    // Returns pointer to the first loop

		case SAB_I2C_REG_LOOP2FX:
			return &self->loop_data[1];    // Returns pointer to the second loop

		case SAB_I2C_REG_LOOP3FX:
			return &self->loop_data[2];    // Returns pointer to the third loop

		case SAB_I2C_REG_LOOP4FX:
			return &self->loop_data[3];    // Returns pointer to the fourth loop

		case SAB_I2C_REG_FXPARAM1:
			return &self->fx_param_un[0];  // Returns pointer to the first FX parameter

		case SAB_I2C_REG_FXPARAM2:
			return &self->fx_param_un[1];  // Returns pointer to the second FX parameter

		case SAB_I2C_REG_FXPARAM3:
			return &self->fx_param_un[2];  // Returns pointer to the third FX parameter

		case SAB_I2C_REG_FXPARAM4:
			return &self->fx_param_un[3];  // Returns pointer to the fourth FX parameter

		case SAB_I2C_REG_FXPARAM5:
			return &self->fx_param_un[4];  // Returns pointer to the fifth FX parameter

		case SAB_I2C_REG_FXPARAM6:
			return &self->fx_param_un[5];  // Returns pointer to the sixth FX parameter

		case SAB_I2C_REG_FXPARAM7:
			return &self->fx_param_un[6];  // Returns pointer to the seventh FX parameter

		case SAB_I2C_REG_FXPARAM8:
			return &self->fx_param_un[7];  // Returns pointer to the eighth FX parameter

		case SAB_I2C_REG_FXPARAM9:
			return &self->fx_param_un[8];  // Returns pointer to the ninth FX parameter

		case SAB_I2C_REG_FXPARAM10:
			return &self->fx_param_un[9];  // Returns pointer to the tenth FX parameter

		case SAB_I2C_REG_FXPARAM11:
			return &self->fx_param_un[10]; // Returns pointer to the eleventh FX parameter

		case SAB_I2C_REG_FXPARAM12:
			return &self->fx_param_un[11]; // Returns pointer to the twelfth FX parameter

		case SAB_I2C_REG_INFO:
			return &self->info_un;          // Returns pointer to the info union

		case SAB_I2C_REG_SAVEPRESET:
			return &self->save_un;          // Returns pointer to the save union

		case SAB_I2C_REG_LOOPBYPASSSTATE:
			return &self->loopbypass_un;    // Returns pointer to the loop bypass union

		default:
			return NULL;  // Return NULL if the register enum is out of bounds
    }	
}

uint8_t sab_intercom_get_reg_data_size(struct sab_intercom_st* self){
	return sizeof(self->get_reg_data_ptr(self));
}

void next_preset(struct sab_intercom_st* self){
	SCB_InvalidateDCache_by_Addr((uint32_t*)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
	if(self->preset_data_un.preset_Minor_u8 == 9 && self->preset_data_un.preset_Major_u8 == 'Z'){
		
	}else{
		self->preset_data_un.preset_Minor_u8++;
		if(self->preset_data_un.preset_Minor_u8 > 9){
			self->preset_data_un.preset_Minor_u8 = 1;
			if(self->preset_data_un.preset_Major_u8<'Z'){
				self->preset_data_un.preset_Major_u8++;
			}
		}
	}
	SCB_CleanDCache_by_Addr((uint32_t*)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
}

void prev_preset(struct sab_intercom_st* self){
	SCB_InvalidateDCache_by_Addr((uint32_t*)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
	if(self->preset_data_un.preset_Minor_u8 == 1 && self->preset_data_un.preset_Major_u8 == 'A'){
	}else{
		self->preset_data_un.preset_Minor_u8--;
		if(self->preset_data_un.preset_Minor_u8 < 1){
			if(self->preset_data_un.preset_Major_u8>'A'){
				self->preset_data_un.preset_Minor_u8 = 9;
				self->preset_data_un.preset_Major_u8--;
			}
		}
	}
	SCB_CleanDCache_by_Addr((uint32_t*)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
}

// ----------------------------------INIT-------------------------------------------------
void init_intercom(struct sab_intercom_st* self, uint8_t slave_address_u8,I2C_HandleTypeDef *i2c_h){

	SCB_InvalidateDCache_by_Addr((uint32_t*)self, sizeof(self));

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

	self->next_preset		= next_preset;
	self->prev_preset       = prev_preset;

	self->process_rx_buffer = sab_intercom_process_i2c_data;
	self->get_reg_data_ptr = sab_intercom_get_reg_data_ptr;
	self->get_reg_data_len = sab_intercom_get_reg_data_size;
	SCB_CleanDCache_by_Addr((uint32_t*)self, sizeof(self));

	testing_data(self);


}

static test_fx_params_fill(struct sab_intercom_st* self){
	strcpy(self->fx_param_un[0].name, "SUB");  // Delay time parameter
		self->fx_param_un[0].type_en = 5;  // Type: Delay
		self->fx_param_un[0].value_u8 = 69;  // Value in ms

		strcpy(self->fx_param_un[1].name, "VOL");  // Modulation rate
		self->fx_param_un[1].type_en = 2;  // Type: Modulation
		self->fx_param_un[1].value_u8 = 120;  // Value in Hz

		strcpy(self->fx_param_un[2].name, "UP");  // Effect depth
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

	SCB_InvalidateDCache_by_Addr((uint32_t*)&(self->preset_data_un.all_u32), sizeof(self->preset_data_un.all_u32));
	self->preset_data_un.preset_Major_u8 = 'A';
	self->preset_data_un.preset_Minor_u8 = 1;
	SCB_CleanDCache_by_Addr((uint32_t*)&(self->preset_data_un.all_u32), sizeof(self->preset_data_un.all_u32));

	// LOOP 1 DATA
	strcpy(self->loop_data[0].slot1.name,"Octaver");
	self->loop_data[0].slot1.color[0] = 255; 	//R
	self->loop_data[0].slot1.color[1] = 0;	//G
	self->loop_data[0].slot1.color[2] = 0;	//B

	strcpy(self->loop_data[0].slot2.name,"Chorus");
	self->loop_data[0].slot2.color[0] = 0;	//R
	self->loop_data[0].slot2.color[1] = 255;	//G
	self->loop_data[0].slot2.color[2] = 0;	//B

	strcpy(self->loop_data->slot3.name,"NONE");

	// LOOP 2 DATA
	strcpy(self->loop_data[1].slot1.name,"Delay");
	self->loop_data[1].slot1.color[0] = 0; 	//R
	self->loop_data[1].slot1.color[1] = 0;	//G
	self->loop_data[1].slot1.color[2] = 255;	//B

	strcpy(self->loop_data[0].slot2.name,"Reverb");
	self->loop_data[1].slot2.color[0] = 0;	//R
	self->loop_data[1].slot2.color[1] = 255;	//G
	self->loop_data[1].slot2.color[2] = 255;	//B

	strcpy(self->loop_data->slot3.name,"NONE");

	self->info_un.dsp_info_st.dsp_fw_ver_major_u8 = 0;
	self->info_un.dsp_info_st.dsp_fw_ver_minor_u8 = 1;
	self->info_un.dsp_info_st.dsp_update_date_day_u8 	= 27;
	self->info_un.dsp_info_st.dsp_update_date_month_u8 	= 10;
	self->info_un.dsp_info_st.dsp_update_date_year_u8 	= 24;
	
	self->loopbypass_un.L12 = 1; // open
	self->loopbypass_un.L23 = 0; // closed
	self->loopbypass_un.L34 = 1; // open
}
