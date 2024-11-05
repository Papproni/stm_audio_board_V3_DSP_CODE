/*
 * sab_intercom.c
 *
 *  Created on: Oct 11, 2024
 *      Author: pappr
 */

#include "sab_intercom.h"

// ----------------------------------GETTERS-------------------------------------------------
static void get_preset_data(struct sab_intercom_st *self)
{
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_PRESETNUM, I2C_MEMADD_SIZE_8BIT,
					 &self->preset_data_un.all_u32, SAB_I2C_REG_PRESETNUM_LEN,
					 1000);
}
static void get_loop_data(struct sab_intercom_st *self, uint8_t loop_num_u8)
{
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_LOOP1FX + loop_num_u8 - 1,
					 I2C_MEMADD_SIZE_8BIT,
					 self->loop_data->all_pau8, SAB_I2C_REG_LOOPFX_LEN,
					 1000);
}
static void get_fx_param(struct sab_intercom_st *self, uint8_t param_slot_u8)
{
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_FXPARAM1 + param_slot_u8 - 1,
					 I2C_MEMADD_SIZE_8BIT,
					 &self->fx_param_pun[self->current_fx_in_edit][param_slot_u8 - 1].all_au8, SAB_I2C_REG_FXPARAM_LEN,
					 1000);
}

static void get_info(struct sab_intercom_st *self)
{
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_INFO,
					 I2C_MEMADD_SIZE_8BIT,
					 &self->info_un.all_u8, SAB_I2C_REG_INFO_LEN,
					 1000);
}
static void get_loopbypass(struct sab_intercom_st *self)
{
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_LOOPBYPASSSTATE,
					 I2C_MEMADD_SIZE_8BIT,
					 &self->loopbypass_un.all_u8, SAB_I2C_REG_LOOPBYPASSSTATE_LEN,
					 1000);
}

static void get_num_of_implemented_effects  (struct sab_intercom_st* self){
	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
						SAB_I2C_REG_NUM_OF_IMPLEMENTED_EFFECTS,
						I2C_MEMADD_SIZE_8BIT,
						&self->num_of_implemented_effects, 1,
						1000);
}

static void get_implemented_effects(struct sab_intercom_st *self)
{
	get_num_of_implemented_effects(self);

	while (!self->num_of_implemented_effects)
	{
	}

	self->implemented_fx_data_ptr = malloc(sizeof(fx_data_tst) * self->num_of_implemented_effects);
	if (NULL == self->implemented_fx_data_ptr)
	{
		return;
	}

	HAL_I2C_Mem_Read(self->i2c_h, self->slave_addr_u8,
					 SAB_I2C_REG_IMPLEMENTED_EFFECTS,
					 I2C_MEMADD_SIZE_8BIT,
					 self->implemented_fx_data_ptr, sizeof(fx_data_tst) * self->num_of_implemented_effects,
					 1000);
}

// ----------------------------------SETTERS-------------------------------------------------
static void set_fx_param(struct sab_intercom_st *self, uint8_t param_slot_u8, uint8_t new_value_u8)
{
	self->fx_param_pun[self->current_fx_in_edit][param_slot_u8 - 1].value_u8 = new_value_u8;

	HAL_I2C_Mem_Write(self->i2c_h, self->slave_addr_u8,
					  SAB_I2C_REG_FXPARAM1 + param_slot_u8 - 1,
					  I2C_MEMADD_SIZE_8BIT,
					  &self->fx_param_pun[self->current_fx_in_edit][param_slot_u8 - 1], SAB_I2C_REG_FXPARAM_LEN, 1000);
}

static void set_loopbypass(struct sab_intercom_st *self)
{
	HAL_I2C_Mem_Write(self->i2c_h, self->slave_addr_u8,
					  SAB_I2C_REG_LOOPBYPASSSTATE,
					  I2C_MEMADD_SIZE_8BIT,
					  &self->loopbypass_un.all_u8, SAB_I2C_REG_LOOPBYPASSSTATE_LEN, 1000);
}

static void set_current_fx_in_edit (struct sab_intercom_st* self,uint8_t fx_slot_u8){
	self->current_fx_in_edit = fx_slot_u8;
	HAL_I2C_Mem_Write(self->i2c_h,self->slave_addr_u8,
				SAB_I2c_REG_CURRENT_FX,
				I2C_MEMADD_SIZE_8BIT,
				&self->current_fx_in_edit, sizeof(uint8_t), 1000);
}

/// @brief Saves data received from master (display)
/// @param self
/// @param buffer_pu8
/// @param size_u8
void sab_intercom_process_i2c_data(struct sab_intercom_st *self, uint8_t *buffer_pu8, uint8_t size_u8)
{
	memcpy(self->get_reg_data_ptr(self), buffer_pu8, size_u8);
}

/// @brief This function returns the data pointer which is adressed by the "register_addr_u8"
/// @param self
/// @return
uint32_t sab_intercom_get_reg_data_ptr(struct sab_intercom_st *self)
{

	switch (self->register_addr_u8)
	{
	case SAB_I2C_REG_PRESETNUM:
		return &self->preset_data_un; // Returns pointer to the preset data union

	case SAB_I2C_REG_LOOP1FX:
		return self->loop_data[0].all_pau8; // Returns pointer to the first loop

	case SAB_I2C_REG_LOOP2FX:
		return self->loop_data[1].all_pau8; // Returns pointer to the second loop

	case SAB_I2C_REG_LOOP3FX:
		return self->loop_data[2].all_pau8; // Returns pointer to the third loop

	case SAB_I2C_REG_LOOP4FX:
		return self->loop_data[3].all_pau8; // Returns pointer to the fourth loop

	case SAB_I2C_REG_FXPARAM1:
		return &self->fx_param_pun[self->current_fx_in_edit][0]; // Returns pointer to the first FX parameter

	case SAB_I2C_REG_FXPARAM2:
		return &self->fx_param_pun[self->current_fx_in_edit][1]; // Returns pointer to the second FX parameter

	case SAB_I2C_REG_FXPARAM3:
		return &self->fx_param_pun[self->current_fx_in_edit][2]; // Returns pointer to the third FX parameter

	case SAB_I2C_REG_FXPARAM4:
		return &self->fx_param_pun[self->current_fx_in_edit][3]; // Returns pointer to the fourth FX parameter

	case SAB_I2C_REG_FXPARAM5:
		return &self->fx_param_pun[self->current_fx_in_edit][4]; // Returns pointer to the fifth FX parameter

	case SAB_I2C_REG_FXPARAM6:
		return &self->fx_param_pun[self->current_fx_in_edit][5]; // Returns pointer to the sixth FX parameter

	case SAB_I2C_REG_FXPARAM7:
		return &self->fx_param_pun[self->current_fx_in_edit][6]; // Returns pointer to the seventh FX parameter

	case SAB_I2C_REG_FXPARAM8:
		return &self->fx_param_pun[self->current_fx_in_edit][7]; // Returns pointer to the eighth FX parameter

	case SAB_I2C_REG_FXPARAM9:
		return &self->fx_param_pun[self->current_fx_in_edit][8]; // Returns pointer to the ninth FX parameter

	case SAB_I2C_REG_FXPARAM10:
		return &self->fx_param_pun[self->current_fx_in_edit][9]; // Returns pointer to the tenth FX parameter

	case SAB_I2C_REG_FXPARAM11:
		return &self->fx_param_pun[self->current_fx_in_edit][10]; // Returns pointer to the eleventh FX parameter

	case SAB_I2C_REG_FXPARAM12:
		return &self->fx_param_pun[self->current_fx_in_edit][11]; // Returns pointer to the twelfth FX parameter

	case SAB_I2C_REG_INFO:
		return &self->info_un; // Returns pointer to the info union

	case SAB_I2C_REG_SAVEPRESET:
		return &self->save_un; // Returns pointer to the save union

	case SAB_I2C_REG_LOOPBYPASSSTATE:
		return &self->loopbypass_un; // Returns pointer to the loop bypass union

	case SAB_I2C_REG_NUM_OF_IMPLEMENTED_EFFECTS:
		return &self->num_of_implemented_effects;

	case SAB_I2C_REG_IMPLEMENTED_EFFECTS:
		return self->implemented_fx_data_ptr;

	case SAB_I2c_REG_CURRENT_FX:
		return &self->current_fx_in_edit;

	default:
		return NULL; // Return NULL if the register enum is out of bounds
	}
}

uint8_t sab_intercom_get_reg_data_size(struct sab_intercom_st *self)
{
	switch (self->register_addr_u8)
	{
	case SAB_I2C_REG_PRESETNUM:
		return sizeof(sab_preset_num_tun); // Returns pointer to the preset data union

	case SAB_I2C_REG_LOOP1FX:
	case SAB_I2C_REG_LOOP2FX:
	case SAB_I2C_REG_LOOP3FX:
	case SAB_I2C_REG_LOOP4FX:
		return sizeof(sab_loop_num_tun); // Returns pointer to the fourth loop

	case SAB_I2C_REG_FXPARAM1:
	case SAB_I2C_REG_FXPARAM2:
	case SAB_I2C_REG_FXPARAM3:
	case SAB_I2C_REG_FXPARAM4:
	case SAB_I2C_REG_FXPARAM5:
	case SAB_I2C_REG_FXPARAM6:
	case SAB_I2C_REG_FXPARAM7:
	case SAB_I2C_REG_FXPARAM8:
	case SAB_I2C_REG_FXPARAM9:
	case SAB_I2C_REG_FXPARAM10:
	case SAB_I2C_REG_FXPARAM11:
	case SAB_I2C_REG_FXPARAM12:
		return sizeof(sab_fx_param_tun); // Returns pointer to the twelfth FX parameter

	case SAB_I2C_REG_INFO:
		return sizeof(sab_info_tun); // Returns pointer to the info union

	case SAB_I2C_REG_SAVEPRESET:
		return sizeof(sab_save_tun); // Returns pointer to the save union

	case SAB_I2C_REG_LOOPBYPASSSTATE:
		return sizeof(sab_loopbypass_tun); // Returns pointer to the loop bypass union

	case SAB_I2C_REG_NUM_OF_IMPLEMENTED_EFFECTS:
		return sizeof(uint8_t); // Returns pointer to the loop bypass union

	case SAB_I2C_REG_IMPLEMENTED_EFFECTS:
		return sizeof(fx_data_tst) * self->num_of_implemented_effects; // Returns pointer to the loop bypass union

	case SAB_I2c_REG_CURRENT_FX:
		return sizeof(uint8_t);

	default:
		return NULL; // Return NULL if the register enum is out of bounds
	}
}

void next_preset(struct sab_intercom_st *self)
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
	if (self->preset_data_un.preset_Minor_u8 == 9 && self->preset_data_un.preset_Major_u8 == 'Z')
	{
	}
	else
	{
		self->preset_data_un.preset_Minor_u8++;
		if (self->preset_data_un.preset_Minor_u8 > 9)
		{
			self->preset_data_un.preset_Minor_u8 = 1;
			if (self->preset_data_un.preset_Major_u8 < 'Z')
			{
				self->preset_data_un.preset_Major_u8++;
			}
		}
	}
	SCB_CleanDCache_by_Addr((uint32_t *)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
}

void prev_preset(struct sab_intercom_st *self)
{
	SCB_InvalidateDCache_by_Addr((uint32_t *)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
	if (self->preset_data_un.preset_Minor_u8 == 1 && self->preset_data_un.preset_Major_u8 == 'A')
	{
	}
	else
	{
		self->preset_data_un.preset_Minor_u8--;
		if (self->preset_data_un.preset_Minor_u8 < 1)
		{
			if (self->preset_data_un.preset_Major_u8 > 'A')
			{
				self->preset_data_un.preset_Minor_u8 = 9;
				self->preset_data_un.preset_Major_u8--;
			}
		}
	}
	SCB_CleanDCache_by_Addr((uint32_t *)&self->preset_data_un.all_u32, sizeof(self->preset_data_un.all_u32));
}

void add_parameter(sab_fx_param_tun* param_ptr, char* name, param_type_ten type, uint8_t value){
	strcpy(param_ptr->name, name);
	param_ptr->type_en = type;	
	param_ptr->value_u8 = value;
}

// ----------------------------------INIT-------------------------------------------------
void init_intercom(struct sab_intercom_st *self, uint8_t slave_address_u8, I2C_HandleTypeDef *i2c_h)
{

	SCB_InvalidateDCache_by_Addr((uint32_t *)self, sizeof(self));

	self->slave_addr_u8 = slave_address_u8 << 1;
	self->i2c_h = i2c_h;
	// GETTER function pointers
	self->get_preset_data = get_preset_data;
	self->get_loop_data = get_loop_data;
	self->get_fx_param = get_fx_param;
	self->get_info = get_info;
	self->get_loopbypass = get_loopbypass;

	self->get_implemented_effects = get_implemented_effects;
	self->num_of_implemented_effects = 0;
	self->implemented_fx_data_ptr = NULL;
	self->current_fx_in_edit = 0;
	// SETTER function pointers
	self->set_fx_param = set_fx_param;
	self->set_loopbypass = set_loopbypass;
	self->set_current_fx_in_edit = set_current_fx_in_edit;
	self->next_preset = next_preset;
	self->prev_preset = prev_preset;

	self->process_rx_buffer = sab_intercom_process_i2c_data;
	self->get_reg_data_ptr = sab_intercom_get_reg_data_ptr;
	self->get_reg_data_len = sab_intercom_get_reg_data_size;
	SCB_CleanDCache_by_Addr((uint32_t *)self, sizeof(self));

	testing_data(self);
}

static test_fx_params_fill(struct sab_intercom_st *self)
{
	// SCB_InvalidateDCache_by_Addr((uint32_t *)(self->fx_param_un->all_au8), sizeof(sab_fx_param_tun)*12);
	// strcpy(self->fx_param_un[0].name, "SUB"); // Delay time parameter
	// self->fx_param_un[0].type_en = PARAM_TYPE_POT;		  // Type: Delay
	// self->fx_param_un[0].value_u8 = 69;		  // Value in ms

	// strcpy(self->fx_param_un[1].name, "VOL"); // Modulation rate
	// self->fx_param_un[1].type_en = PARAM_TYPE_POT;		  // Type: Modulation
	// self->fx_param_un[1].value_u8 = 120;	  // Value in Hz

	// strcpy(self->fx_param_un[2].name, "UP"); // Effect depth
	// self->fx_param_un[2].type_en = PARAM_TYPE_POT;		 // Type: Modulation Depth
	// self->fx_param_un[2].value_u8 = 85;		 // Percentage

	// strcpy(self->fx_param_un[3].name, "GAIN"); // Amplification gain
	// self->fx_param_un[3].type_en = PARAM_TYPE_UNUSED;		   // Type: Amplification
	// self->fx_param_un[3].value_u8 = 100;	   // Value in dB

	// strcpy(self->fx_param_un[4].name, "MIX"); // Dry/Wet mix
	// self->fx_param_un[4].type_en = PARAM_TYPE_UNUSED;		  // Type: Mix
	// self->fx_param_un[4].value_u8 = 50;		  // 50% mix

	// strcpy(self->fx_param_un[5].name, "DECAY"); // Reverb decay
	// self->fx_param_un[5].type_en = PARAM_TYPE_UNUSED;			// Type: Reverb
	// self->fx_param_un[5].value_u8 = 72;			// Value in percentage

	// strcpy(self->fx_param_un[6].name, "FREQ"); // Filter cutoff frequency
	// self->fx_param_un[6].type_en = PARAM_TYPE_UNUSED;		   // Type: Filter
	// self->fx_param_un[6].value_u8 = 200;	   // Frequency in Hz

	// strcpy(self->fx_param_un[7].name, "RES"); // Filter resonance
	// self->fx_param_un[7].type_en = PARAM_TYPE_UNUSED;		  // Type: Filter
	// self->fx_param_un[7].value_u8 = 60;		  // Resonance value

	// strcpy(self->fx_param_un[8].name, "LEVEL"); // Output level
	// self->fx_param_un[8].type_en = PARAM_TYPE_UNUSED;			// Type: Amplification
	// self->fx_param_un[8].value_u8 = 95;			// Value in dB

	// strcpy(self->fx_param_un[9].name, "FEED"); // Feedback amount
	// self->fx_param_un[9].type_en = PARAM_TYPE_UNUSED;		   // Type: Delay/Feedback
	// self->fx_param_un[9].value_u8 = 40;		   // Percentage

	// strcpy(self->fx_param_un[10].name, "THRSH"); // Compression threshold
	// self->fx_param_un[10].type_en = PARAM_TYPE_UNUSED;			 // Type: Compression
	// self->fx_param_un[10].value_u8 = 128;		 // Threshold level

	// strcpy(self->fx_param_un[11].name, "SPD"); // Tremolo speed
	// self->fx_param_un[11].type_en = PARAM_TYPE_UNUSED;		   // Type: Tremolo
	// self->fx_param_un[11].value_u8 = 90;	   // Speed in Hz

	// SCB_CleanDCache_by_Addr((uint32_t *)(self->fx_param_un->all_au8), sizeof(sab_fx_param_tun)*12);
}
void testing_data(struct sab_intercom_st *self)
{
	test_fx_params_fill(self);

	SCB_InvalidateDCache_by_Addr((uint32_t *)&(self->preset_data_un.all_u32), sizeof(self->preset_data_un.all_u32));
	self->preset_data_un.preset_Major_u8 = 'A';
	self->preset_data_un.preset_Minor_u8 = 1;
	SCB_CleanDCache_by_Addr((uint32_t *)&(self->preset_data_un.all_u32), sizeof(self->preset_data_un.all_u32));

	SCB_InvalidateDCache_by_Addr((uint32_t *)&(self->loop_data[0]), sizeof(self->loop_data));
	// LOOP 1 DATA
	// strcpy(self->loop_data[0].slot1.name, "Octaver");
	// self->loop_data[0].slot1.color[0] = 255; // R
	// self->loop_data[0].slot1.color[1] = 0;	 // G
	// self->loop_data[0].slot1.color[2] = 0;	 // B

	// strcpy(self->loop_data[0].slot2.name, "Chorus");
	// self->loop_data[0].slot2.color[0] = 0;	 // R
	// self->loop_data[0].slot2.color[1] = 255; // G
	// self->loop_data[0].slot2.color[2] = 0;	 // B

	// strcpy(self->loop_data->slot3.name, "NONE");

	SCB_CleanDCache_by_Addr((uint32_t *)&(self->loop_data[0]), sizeof(self->loop_data));

	// LOOP 2 DATA
	SCB_InvalidateDCache_by_Addr((uint32_t *)&(self->loop_data[1]), sizeof(self->loop_data));
	// strcpy(self->loop_data[1].slot1.name, "Delay");
	// self->loop_data[1].slot1.color[0] = 0;	 // R
	// self->loop_data[1].slot1.color[1] = 0;	 // G
	// self->loop_data[1].slot1.color[2] = 255; // B

	// strcpy(self->loop_data[0].slot2.name, "Reverb");
	// self->loop_data[1].slot2.color[0] = 0;	 // R
	// self->loop_data[1].slot2.color[1] = 255; // G
	// self->loop_data[1].slot2.color[2] = 255; // B

	// strcpy(self->loop_data->slot3.name, "NONE");
	// SCB_CleanDCache_by_Addr((uint32_t *)&(self->loop_data[1]), sizeof(self->loop_data));

	SCB_InvalidateDCache_by_Addr((uint32_t *)&(self->info_un), sizeof(self->info_un));
	self->info_un.dsp_info_st.dsp_fw_ver_major_u8 = 0;
	self->info_un.dsp_info_st.dsp_fw_ver_minor_u8 = 1;
	self->info_un.dsp_info_st.dsp_update_date_day_u8 = 27;
	self->info_un.dsp_info_st.dsp_update_date_month_u8 = 10;
	self->info_un.dsp_info_st.dsp_update_date_year_u8 = 24;
	SCB_CleanDCache_by_Addr((uint32_t *)&(self->info_un), sizeof(self->info_un));

	self->loopbypass_un.L12 = 1; // open
	self->loopbypass_un.L23 = 0; // closed
	self->loopbypass_un.L34 = 1; // open

	self->num_of_implemented_effects = 5;
	self->implemented_fx_data_ptr = (fx_data_tst *)malloc(sizeof(fx_data_tst) * self->num_of_implemented_effects);
	
	strcpy(self->implemented_fx_data_ptr[0].name, "BOOST");
	self->implemented_fx_data_ptr[0].color[0] = 0;
	self->implemented_fx_data_ptr[0].color[1] = 200;
	self->implemented_fx_data_ptr[0].color[2] = 100;

	strcpy(self->implemented_fx_data_ptr[1].name, "OVERDRIVE");
	self->implemented_fx_data_ptr[1].color[0] = 255;
	self->implemented_fx_data_ptr[1].color[1] = 100;
	self->implemented_fx_data_ptr[1].color[2] = 50;

	strcpy(self->implemented_fx_data_ptr[2].name, "DELAY");
	self->implemented_fx_data_ptr[2].color[0] = 0;
	self->implemented_fx_data_ptr[2].color[1] = 150;
	self->implemented_fx_data_ptr[2].color[2] = 255;

	strcpy(self->implemented_fx_data_ptr[3].name, "REVERB");
	self->implemented_fx_data_ptr[3].color[0] = 100;
	self->implemented_fx_data_ptr[3].color[1] = 100;
	self->implemented_fx_data_ptr[3].color[2] = 255;

	strcpy(self->implemented_fx_data_ptr[4].name, "CHORUS");
	self->implemented_fx_data_ptr[4].color[0] = 50;
	self->implemented_fx_data_ptr[4].color[1] = 255;
	self->implemented_fx_data_ptr[4].color[2] = 150;
}
