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

static void get_num_of_implemented_effects(struct sab_intercom_st *self)
{
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

static void set_current_fx_in_edit(struct sab_intercom_st *self, uint8_t fx_slot_u8)
{
	self->current_fx_in_edit = fx_slot_u8;
	HAL_I2C_Mem_Write(self->i2c_h, self->slave_addr_u8,
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
	self->change_occured_flg = self->register_addr_u8;
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
		return &self->loop_data[0]; // Returns pointer to the first loop

	case SAB_I2C_REG_LOOP2FX:
		return &self->loop_data[1]; // Returns pointer to the second loop

	case SAB_I2C_REG_LOOP3FX:
		return &self->loop_data[2]; // Returns pointer to the third loop

	case SAB_I2C_REG_LOOP4FX:
		return &self->loop_data[3]; // Returns pointer to the fourth loop

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
	
	case SAB_I2C_REG_DSP_FW_UPDATE:
		return &self->dsp_fw_update_flg;

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
		
	case SAB_I2C_REG_DSP_FW_UPDATE:
		return sizeof(uint8_t);

	default:
		return NULL; // Return NULL if the register enum is out of bounds
	}
}

float32_t conv_raw_to_param_value(uint8_t value, float32_t low, float32_t high) {
	float32_t v_f32 = value;
    return low + (v_f32 / 255.0f) * (high - low);
}

void add_parameter(sab_fx_param_tun *param_ptr, char *name, param_type_ten type, uint8_t value)
{
	strcpy(param_ptr->name, name);
	param_ptr->type_en = type;
	param_ptr->value_u8 = value;
}


// Macro to statically define FX data
#define DEFINE_FX_DATA(...) \
    const fx_data_tst IMPLEMENTED_FX_DATA[] = { __VA_ARGS__ }; \
    const size_t IMPLEMENTED_FX_DATA_COUNT = sizeof(IMPLEMENTED_FX_DATA) / sizeof(IMPLEMENTED_FX_DATA[0]);

// Use the macro to define your FX data
DEFINE_FX_DATA(
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Octave",     	.color = { 0, 150, 255 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "CUSTOM_FX",    .color = { 0, 150, 255 } },
    { .fx_state_en = FX_STATE_OFF,  .name = "Delay", 		.color = { 255, 100, 50 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Boost",     	.color = { 0, 200, 100 } },
    { .fx_state_en = FX_STATE_OFF,  .name = "Chorus", 		.color = { 255, 100, 50 } },
    { .fx_state_en = FX_STATE_OFF, 	.name = "Custom_fx",    .color = { 0, 150, 255 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Dist",   		.color = { 0, 200, 100 } },
    { .fx_state_en = FX_STATE_OFF,  .name = "Overdrv", 		.color = { 255, 100, 50 } },
    { .fx_state_en = FX_STATE_OFF, 	.name = "EQ",    		.color = { 0, 150, 255 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Flanger",     	.color = { 0, 200, 100 } },
    { .fx_state_en = FX_STATE_OFF, 	.name = "Fuzz",     	.color = { 0, 150, 255 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Tremolo",     	.color = { 0, 150, 255 } },
	{ .fx_state_en = FX_STATE_OFF, 	.name = "Pitchshft",    .color = { 0, 150, 255 } }
);

// ----------------------------------INIT-------------------------------------------------
void init_intercom(struct sab_intercom_st *self, uint8_t slave_address_u8, I2C_HandleTypeDef *i2c_h)
{

//	SCB_InvalidateDCache_by_Addr((uint32_t *)self, sizeof(self));

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
	self->save_un.save_command = 0;
	// SETTER function pointers
	self->set_fx_param = set_fx_param;
	self->set_loopbypass = set_loopbypass;
	self->set_current_fx_in_edit = set_current_fx_in_edit;

	self->process_rx_buffer = sab_intercom_process_i2c_data;
	self->get_reg_data_ptr = sab_intercom_get_reg_data_ptr;
	self->get_reg_data_len = sab_intercom_get_reg_data_size;
	self->change_occured_flg = 0;

	self->dsp_fw_update_flg = 0;

	self->preset_data_un.preset_Major_u8 = 'A';
	self->preset_data_un.preset_Minor_u8 = 1;

	for (int i = 0; i < (NUM_OF_LOOPS); i++)
	{
		strcpy(self->loop_data[i].slot1.name,"NONE\0");
		strcpy(self->loop_data[i].slot2.name,"NONE\0");
		strcpy(self->loop_data[i].slot3.name,"NONE\0");
	}

	self->info_un.dsp_info_st.dsp_fw_ver_major_u8 = 0;
	self->info_un.dsp_info_st.dsp_fw_ver_minor_u8 = 1;
	self->info_un.dsp_info_st.dsp_update_date_day_u8 = 27;
	self->info_un.dsp_info_st.dsp_update_date_month_u8 = 10;
	self->info_un.dsp_info_st.dsp_update_date_year_u8 = 24;

	self->loopbypass_un.L12 = 1; // open
	self->loopbypass_un.L23 = 0; // closed
	self->loopbypass_un.L34 = 1; // open

	self->num_of_implemented_effects = IMPLEMENTED_FX_DATA_COUNT;
	self->implemented_fx_data_ptr = IMPLEMENTED_FX_DATA;
}


