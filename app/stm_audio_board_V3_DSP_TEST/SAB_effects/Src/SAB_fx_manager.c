/*
 * SAB_fx_manager.c
 *
 *  Created on: Nov 10, 2024
 *      Author: pappr
 */

#include "SAB_fx_manager.h"
#include "stdint.h"

const GuitarEffect DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN = {
     .init = NULL,
     .process = NULL,
     .delete = NULL,
     .intercom_fx_data.name = "NONE"
 };


void init_effect_chain(GuitarEffect** chain, EffectType* fx_chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        switch (fx_chain[i]) {
        case DELAY:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_delay_tst));
            chain[i]->init = SAB_delay_init;
            chain[i]->process = SAB_delay_process;
            chain[i]->delete  = SAB_delay_delete;
            break;
        case OVERDRIVE:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_overdrive_tst));
            chain[i]->init = SAB_overdrive_init;
            chain[i]->process = SAB_overdrive_process;
            chain[i]->delete  = SAB_overdrive_delete;
            break;
        case DISTORTION:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_distortion_tst));
            chain[i]->init = SAB_distortion_init;
            chain[i]->process = SAB_distortion_process;
            chain[i]->delete  = SAB_distortion_delete;
            break;
        case OCTAVE:
            chain[i] = (GuitarEffect*)malloc(sizeof(octave_effects_tst));
            chain[i]->init = SAB_octave_init;
            chain[i]->process = SAB_octave_process;
            chain[i]->delete  = SAB_octave_delete;
            break;
        case FLANGER:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_flanger_tst));
            chain[i]->init = SAB_flanger_init;
            chain[i]->process = SAB_flanger_process;
            chain[i]->delete  = SAB_flanger_delete;
            break;
        case CHORUS:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_chorus_tst));
            chain[i]->init = SAB_chorus_init;
            chain[i]->process = SAB_chorus_process;
            chain[i]->delete  = SAB_chorus_delete;
            break;
        // case BOOST:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_boost_tst));
        //     chain[i]->init = SAB_boost_init;
        //     chain[i]->process = SAB_boost_process;
        //     chain[i]->delete  = SAB_boost_delete;
        //     break;
        // case PITCHSHIFT:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_pitchshift_tst));
        //     chain[i]->init = SAB_pitchshift_init;
        //     chain[i]->process = SAB_pitchshift_process;
        //     chain[i]->delete  = SAB_pitchshift_delete;
        //     break;
        // case FUZZ:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_fuzz_tst));
        //     chain[i]->init = SAB_fuzz_init;
        //     chain[i]->process = SAB_fuzz_process;
        //     chain[i]->delete  = SAB_fuzz_delete;
        //     break;
        // case TREMOLO:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_tremolo_tst));
        //     chain[i]->init = SAB_tremolo_init;
        //     chain[i]->process = SAB_tremolo_process;
        //     chain[i]->delete  = SAB_tremolo_delete;
        //     break;
        // case EQUALIZER:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_equalizer_tst));
        //     chain[i]->init = SAB_equalizer_init;
        //     chain[i]->process = SAB_equalizer_process;
        //     chain[i]->delete  = SAB_equalizer_delete;
        //     break;
        // case CUSTOM:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_CUSTOM_tst));
        //     chain[i]->init = SAB_CUSTOM_init;
        //     chain[i]->process = SAB_CUSTOM_process;
        //     chain[i]->delete  = SAB_CUSTOM_delete;
        //     break;
        case CUSTOM_FX:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_custom_fx_tst));
            chain[i]->init = SAB_custom_fx_init;
            chain[i]->process = SAB_custom_fx_process;
            chain[i]->delete  = SAB_custom_fx_delete;
            break;
        default:
            // TODO: ADD DUMMY DATA DEFINE HERE
             chain[i] = &DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN;
            break;
        }
        // Initialize the effect
        if(NULL != chain[i]->init){
            chain[i]->init(chain[i]);
        }
        
    }
}

EffectType get_fx_type(char* fx_name_char) {
    if (strcmp(fx_name_char, "Delay") == 0) {
        return DELAY;
    } else if (strcmp(fx_name_char, "Octave") == 0) {
        return OCTAVE;
    } else if (strcmp(fx_name_char, "CUSTOM_FX") == 0) {
        return CUSTOM_FX;
    } else if (strcmp(fx_name_char, "Overdrive") == 0) {
        return OVERDRIVE;
    } else if (strcmp(fx_name_char, "Distortion") == 0) {
        return DISTORTION;
    } else if (strcmp(fx_name_char, "Flanger") == 0) {
        return FLANGER;
    } else if (strcmp(fx_name_char, "Chorus") == 0) {
        return CHORUS;
    } else {
        // Handle unknown effect types
        // You might return a default value or handle it as an error
        // For example:
        return -1; // Invalid enum value
    }
}


// Function to process all effects in the chain
void SAB_process_effect_chain(SAB_fx_manager_tst* self, GuitarEffect** chain, int chain_length) {
    // --------------------------------- LOOP 1 ------------------
    float32_t data_sample_f32=(float32_t)self->hardware_IO_port->in1_i32;
    fx_active_modes_ten mode = self->preset_mode_st.preset_mode_en;
    uint8_t bypass_L12 = self->current_preset_config_st.bypass_states_st[mode].loop_bypass_un.L12;
    uint8_t bypass_L23 = self->current_preset_config_st.bypass_states_st[mode].loop_bypass_un.L23;
    uint8_t bypass_L34 = self->current_preset_config_st.bypass_states_st[mode].loop_bypass_un.L34;
    for(int i=0; i<3; i++){
        if(chain[i]->process != NULL && chain[i]->intercom_fx_data.fx_state_en == FX_STATE_ON){
            data_sample_f32 = chain[i]->process(chain[i],data_sample_f32);
        }
    }
    if(0 != bypass_L12){
        self->hardware_IO_port->out1_i32 = data_sample_f32;
        data_sample_f32 = self->hardware_IO_port->in2_i32;
    }
    // --------------------------------- LOOP 2 ------------------
    for(int i=3; i<6; i++){
        if(chain[i]->process != NULL && chain[i]->intercom_fx_data.fx_state_en == FX_STATE_ON){
            data_sample_f32 = chain[i]->process(chain[i],data_sample_f32);
        }
    }
    if(0 != bypass_L23){
        self->hardware_IO_port->out2_i32 = data_sample_f32;
        data_sample_f32 = self->hardware_IO_port->in3_i32;
    }
    // --------------------------------- LOOP 3 ------------------
    for(int i=6; i<9; i++){
        if(chain[i]->process != NULL && chain[i]->intercom_fx_data.fx_state_en == FX_STATE_ON){
            data_sample_f32 = chain[i]->process(chain[i],data_sample_f32);
        }
    }
    if(0 != bypass_L34){
        self->hardware_IO_port->out3_i32 = data_sample_f32;
        data_sample_f32 = self->hardware_IO_port->in4_i32;
    }
    // --------------------------------- LOOP 4 ------------------
    for(int i=9; i<12; i++){
        if(chain[i]->process != NULL && chain[i]->intercom_fx_data.fx_state_en == FX_STATE_ON){
            data_sample_f32 = chain[i]->process(chain[i],data_sample_f32);
        }
    }
    self->hardware_IO_port->out4_i32 = data_sample_f32;
}

// if relevant data came on I2C handle it (fx change, add delete etc)
static uint8_t SAB_handle_intercom_change(SAB_fx_manager_tst* self){
    uint8_t register_u8 = self->intercom_pst->change_occured_flg;
    self->intercom_pst->change_occured_flg = 0;
    fx_active_modes_ten current_mode = self->preset_mode_st.preset_mode_en;
    uint8_t reinit_needed_flg = 0;

    fx_data_tst *slot_ptr;
    preset_fx_bypasses_tst* current_bypass = &self->current_preset_config_st.bypass_states_st[current_mode];

    switch (register_u8)
    {
        case SAB_I2C_REG_LOOP1FX:
        case SAB_I2C_REG_LOOP2FX:
        case SAB_I2C_REG_LOOP3FX:
        case SAB_I2C_REG_LOOP4FX:
            // Check if name changed
            slot_ptr= &self->intercom_pst->loop_data[0].slot1;
            for(int i=0;i<12;i++){
                if(0 != strcmp(slot_ptr[i].name, self->fx_instances[i]->intercom_fx_data.name)){
                    reinit_needed_flg|=1;
                    strcpy(self->current_preset_config_st.fx_names[i],slot_ptr[i].name);
                }else{
                     // update fx_state
                	if( self->fx_instances[i]->init){
                        self->fx_instances[i]->intercom_fx_data.fx_state_en = slot_ptr[i].fx_state_en;
                        current_bypass->fx_states_aen[i] = slot_ptr[i].fx_state_en;
                	}
                }
            }
            break;

        case SAB_I2C_REG_LOOPBYPASSSTATE:
            memcpy(&current_bypass->loop_bypass_un,&self->intercom_pst->loopbypass_un, sizeof(sab_loopbypass_tun));
            break;
        default:
            break;
    }
    return reinit_needed_flg;
}



// Cleanup the effects in the chain
static void SAB_cleanup_effect_chain(GuitarEffect** chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        if(chain[i]->init != NULL){
            chain[i]->delete(chain[i]);  // Free the allocated memory for each effect
            free(chain[i]);
            chain[i] = &DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN;
        }
    }
}

void SAB_fx_manager_deinit( SAB_fx_manager_tst* self){
    
}

#define USER_FLASH_ADDRESS 0x08080000  // Base address of USER_FLASH section

void SAB_save_preset_to_flash(SAB_fx_manager_tst* self){
	// preset_saves_tst preset_flash_st;
	// 1. Calculate data addr
	uint32_t preset_save_size_in_byte_u32 = 	sizeof(preset_saves_tst);
	uint32_t preset_save_size_in_word_u32 = 	preset_save_size_in_byte_u32 / 4;
	uint32_t preset_num_u32 = (self->intercom_pst->preset_data_un.preset_Major_u8-'A')*9+self->intercom_pst->preset_data_un.preset_Minor_u8-1;
	uint32_t save_location_addr_u32 = preset_save_size_in_byte_u32*preset_num_u32+USER_FLASH_ADDRESS;
    // 2.1. fx names [NEW]
    for(int i=0;i<12;i++){
        memcpy(&self->current_preset_config_st.fx_names[i],&self->fx_instances[i]->intercom_fx_data.name,sizeof(char[10]));
    }
	// 2.2. fx params values
	// choose fx
    for(int i = 0; i<12;i++){
        // choose params
        for(int j=0;j<12;j++){
            self->current_preset_config_st.fx_params_value[i][j] = self->fx_instances[i]->intercom_parameters_aun[j].value_u8;
        }
       
	}
	// 3. save to flash
    __disable_irq();  // Disable interrupts
	Flash_Write_Data(save_location_addr_u32,&self->current_preset_config_st,preset_save_size_in_word_u32);
	__enable_irq();   // Re-enable interrupts
}

void SAB_load_preset_from_flash(SAB_fx_manager_tst* self){
    uint32_t preset_save_size_in_byte_u32 = 	sizeof(preset_saves_tst);
	uint32_t preset_save_size_in_word_u32 = 	preset_save_size_in_byte_u32 / 4;
	uint32_t preset_num_u32 = (self->intercom_pst->preset_data_un.preset_Major_u8-'A')*9+self->intercom_pst->preset_data_un.preset_Minor_u8-1;
	uint32_t save_location_addr_u32 = preset_save_size_in_byte_u32*preset_num_u32+USER_FLASH_ADDRESS;
	__disable_irq();  // Disable interrupts
    Flash_Read_Data(save_location_addr_u32, &self->current_preset_config_st, preset_save_size_in_word_u32);
    __enable_irq();   // Re-enable interrupts
}


static void SAB_load_current_config(SAB_fx_manager_tst* self ){
    for(int i=0; i<12;i++){
        self->fx_types_chain[i] = get_fx_type(self->current_preset_config_st.fx_names[i]);
    }

    init_effect_chain(&self->fx_instances,self->fx_types_chain,12);

    // PARAMS ASSIGN / BYPASS SWITCHES SET
    self->preset_mode_st.preset_mode_en = PRESET_MODE_NORMAL;
    for(int i=0; i<12;i++){
        if(self->fx_instances[i]->init != NULL){
            self->intercom_pst->fx_param_pun[i] = self->fx_instances[i]->intercom_parameters_aun;
        }
    }

// Display data assign
    for(int i=0; i<4;i++){
        memcpy(&self->intercom_pst->loop_data[i].slot1, &self->fx_instances[i*3+0]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&self->intercom_pst->loop_data[i].slot2, &self->fx_instances[i*3+1]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&self->intercom_pst->loop_data[i].slot3, &self->fx_instances[i*3+2]->intercom_fx_data,sizeof(fx_data_tst));
    }
}   

// THis init i used the first time
void SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst, SAB_IO_HADRWARE_BUFFERS* hardware_IO_port_ptr, uint8_t* fsw1_ptr, uint8_t* fsw2_ptr){
    self->intercom_pst      		= intercom_pst;
    self->hardware_IO_port  		= hardware_IO_port_ptr;
    self->preset_mode_st.fsw1_ptr 	= fsw1_ptr;
    self->preset_mode_st.fsw2_ptr 	= fsw2_ptr;

    // LOAD DATA FROM FLASH
    SAB_load_preset_from_flash(self);
    SAB_load_current_config(self);
    
}



void SAB_fsw_pressed_callback(SAB_fx_manager_tst* self){
    uint8_t fsw1 = *(self->preset_mode_st.fsw1_ptr); // USED FOR MODE A
    uint8_t fsw2 = *(self->preset_mode_st.fsw2_ptr); // USED FOR MODE B

//    save_current_mode(self);
    switch (self->preset_mode_st.preset_mode_en)
    {
    case PRESET_MODE_NORMAL:
        if(fsw1) self->preset_mode_st.preset_mode_en = PRESET_MODE_A_ACTIVE;
        if(fsw2) self->preset_mode_st.preset_mode_en = PRESET_MODE_B_ACTIVE;
        /* code */
        break;
    case PRESET_MODE_A_ACTIVE:
        if(fsw1) self->preset_mode_st.preset_mode_en = PRESET_MODE_NORMAL;
        if(fsw2) self->preset_mode_st.preset_mode_en = PRESET_MODE_B_ACTIVE;
        break;
    case PRESET_MODE_B_ACTIVE:
        if(fsw1) self->preset_mode_st.preset_mode_en = PRESET_MODE_A_ACTIVE;
        if(fsw2) self->preset_mode_st.preset_mode_en = PRESET_MODE_NORMAL;
        break;
    default:
        self->preset_mode_st.preset_mode_en = PRESET_MODE_NORMAL;
        break;
    }

    uint8_t current_mode = self->preset_mode_st.preset_mode_en;
    fx_data_tst *slot_ptr= &self->intercom_pst->loop_data[0].slot1;
    // update fx states
    for(int i=0; i<12;i++){
        self->fx_instances[i]->intercom_fx_data.fx_state_en = self->current_preset_config_st.bypass_states_st[current_mode].fx_states_aen[i];
        slot_ptr[i].fx_state_en 							= self->current_preset_config_st.bypass_states_st[current_mode].fx_states_aen[i];
    }
    // update loops
    memcpy(&self->intercom_pst->loopbypass_un,&self->current_preset_config_st.bypass_states_st[current_mode].loop_bypass_un,sizeof(sab_loopbypass_tun));
}

void SAB_preset_up_pressed(SAB_fx_manager_tst* self){
    
    if (self->intercom_pst->preset_data_un.preset_Minor_u8 == 9 && self->intercom_pst->preset_data_un.preset_Major_u8 == 'Z')
	{
	}
	else
	{
		self->intercom_pst->preset_data_un.preset_Minor_u8++;
		if (self->intercom_pst->preset_data_un.preset_Minor_u8 > 9)
		{
			self->intercom_pst->preset_data_un.preset_Minor_u8 = 1;
			if (self->intercom_pst->preset_data_un.preset_Major_u8 < 'Z')
			{
				self->intercom_pst->preset_data_un.preset_Major_u8++;
			}
		}
        // LOAD PRESET DATA FROM FLASH
        SAB_load_preset_from_flash(self);
        // DELETE CURRENT PRESET
        SAB_cleanup_effect_chain(self,12);
        // INIT NEW PRESET
        SAB_load_current_config(self);
	}
}
void SAB_preset_down_pressed(SAB_fx_manager_tst* self){
    if (self->intercom_pst->preset_data_un.preset_Minor_u8 == 1 && self->intercom_pst->preset_data_un.preset_Major_u8 == 'A')
	{
	}
	else
	{
		self->intercom_pst->preset_data_un.preset_Minor_u8--;
		if (self->intercom_pst->preset_data_un.preset_Minor_u8 < 1)
		{
			if (self->intercom_pst->preset_data_un.preset_Major_u8 > 'A')
			{
				self->intercom_pst->preset_data_un.preset_Minor_u8 = 9;
				self->intercom_pst->preset_data_un.preset_Major_u8--;
			}
		}
        // LOAD PRESET DATA FROM FLASH
        SAB_load_preset_from_flash(self);
        // DELETE CURRENT PRESET
        SAB_cleanup_effect_chain(self,12);
        // INIT NEW PRESET
        SAB_load_current_config(self);
	}
}

void SAB_fx_manager_process( SAB_fx_manager_tst* self){
    if(self->intercom_pst->change_occured_flg != 0){
        if(SAB_handle_intercom_change(self)){
            SAB_cleanup_effect_chain(self->fx_instances,12);
            SAB_load_current_config(self);
        }
    }
    SAB_process_effect_chain(self,self->fx_instances,12);
}
