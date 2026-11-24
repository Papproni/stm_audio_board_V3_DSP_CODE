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


/* Per-slot allocation telemetry: how many bytes this slot's effect struct
   occupies on the C heap. A slot with no effect assigned reports 0. The
   sizeof() values mirror the malloc() calls in init_effect_chain() exactly,
   so this is the true footprint the FX manager allocated for that slot
   (octave ~41 KB — invisible to the SDRAM sample-buffer pool). */
uint32_t SAB_fx_slot_size_bytes(const SAB_fx_manager_tst *m, int slot)
{
    if ((m == NULL) || (slot < 0) || (slot >= 12)) {
        return 0;
    }
    const GuitarEffect *p = m->fx_instances[slot];
    if ((p == NULL) || (p == &DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN)) {
        return 0;
    }
    switch (m->fx_types_chain[slot]) {
        case DELAY:         return (uint32_t)sizeof(SAB_delay_tst);
        case OVERDRIVE:     return (uint32_t)sizeof(SAB_overdrive_tst);
        case DISTORTION:    return (uint32_t)sizeof(SAB_distortion_tst);
        case OCTAVE:        return (uint32_t)sizeof(octave_effects_tst);
        case FLANGER:       return (uint32_t)sizeof(SAB_flanger_tst);
        case CHORUS:        return (uint32_t)sizeof(SAB_chorus_tst);
        case BOOST:         return (uint32_t)sizeof(SAB_boost_tst);
        case PITCHSHIFT:    return (uint32_t)sizeof(SAB_pitchshift_tst);
        case FUZZ:          return (uint32_t)sizeof(SAB_fuzz_tst);
        case TREMOLO:       return (uint32_t)sizeof(SAB_tremolo_tst);
        case REVERB:        return (uint32_t)sizeof(SAB_reverb_tst);
        case BANDPASS:      return (uint32_t)sizeof(SAB_bandpass_tst);
        case ENVELOPE:      return (uint32_t)sizeof(SAB_envelope_tst);
        case CUSTOM_FX:     return (uint32_t)sizeof(SAB_custom_fx_tst);
        case PITCHDETECTOR: return (uint32_t)sizeof(SAB_pitchdetector_tst);
        case SIG_GEN:       return (uint32_t)sizeof(SAB_generator_tst);
        case LPF:           return (uint32_t)sizeof(SAB_lpf_tst);
        case HPF:           return (uint32_t)sizeof(SAB_hpf_tst);
        case MARK2C:        return (uint32_t)sizeof(SAB_mark2c_tst);
        case IMRE:          return (uint32_t)sizeof(SAB_step_response_tst);
        case HARMONIZER:    return (uint32_t)sizeof(SAB_harmonizer_tst);
        default:            return 0U;
    }
}

inline void init_effect_chain(GuitarEffect** chain, EffectType* fx_chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        switch (fx_chain[i]) {
        case DELAY:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_delay_tst));
            chain[i]->init = (void*)SAB_delay_init;
            chain[i]->process = (void*)SAB_delay_process;
            chain[i]->delete  = (void*)SAB_delay_delete;
            break;
        case OVERDRIVE:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_overdrive_tst));
            chain[i]->init = (void*)SAB_overdrive_init;
            chain[i]->process = (void*)SAB_overdrive_process;
            chain[i]->delete  = (void*)SAB_overdrive_delete;
            break;
        case DISTORTION:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_distortion_tst));
            chain[i]->init = (void*)SAB_distortion_init;
            chain[i]->process = (void*)SAB_distortion_process;
            chain[i]->delete  = (void*)SAB_distortion_delete;
            break;
        case OCTAVE:
            chain[i] = (GuitarEffect*)malloc(sizeof(octave_effects_tst));
            chain[i]->init = (void*)SAB_octave_init;
            chain[i]->process = (void*)SAB_octave_process;
            chain[i]->delete  = (void*)SAB_octave_delete;
            break;
        case FLANGER:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_flanger_tst));
            chain[i]->init = (void*)SAB_flanger_init;
            chain[i]->process = (void*)SAB_flanger_process;
            chain[i]->delete  = (void*)SAB_flanger_delete;
            break;
        case CHORUS:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_chorus_tst));
            chain[i]->init = (void*)SAB_chorus_init;
            chain[i]->process = (void*)SAB_chorus_process;
            chain[i]->delete  = (void*)SAB_chorus_delete;
            break;
        case BOOST:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_boost_tst));
            chain[i]->init = (void*)SAB_boost_init;
            chain[i]->process = (void*)SAB_boost_process;
            chain[i]->delete  = (void*)SAB_boost_delete;
            break;
        case PITCHSHIFT:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_pitchshift_tst));
            chain[i]->init = (void*)SAB_pitchshift_init;
            chain[i]->process = (void*)SAB_pitchshift_process;
            chain[i]->delete  = (void*)SAB_pitchshift_delete;
            break;
         case FUZZ:
             chain[i] = (GuitarEffect*)malloc(sizeof(SAB_fuzz_tst));
             chain[i]->init = (void*)SAB_fuzz_init;
             chain[i]->process = (void*)SAB_fuzz_process;
             chain[i]->delete  = (void*)SAB_fuzz_delete;
             break;
        case TREMOLO:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_tremolo_tst));
            chain[i]->init = (void*)SAB_tremolo_init;
            chain[i]->process = (void*)SAB_tremolo_process;
            chain[i]->delete  = (void*)SAB_tremolo_delete;
            break;
        case REVERB:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_reverb_tst));
            chain[i]->init = (void*)SAB_reverb_init;
            chain[i]->process = (void*)SAB_reverb_process;
            chain[i]->delete  = (void*)SAB_reverb_delete;
            break;
        case BANDPASS:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_bandpass_tst));
            chain[i]->init = (void*)SAB_bandpass_init;
            chain[i]->process = (void*)SAB_bandpass_process;
            chain[i]->delete  = (void*)SAB_bandpass_delete;
            break;
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
        case ENVELOPE:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_envelope_tst));
            chain[i]->init = (void*)SAB_envelope_init;
            chain[i]->process = (void*)SAB_envelope_process;
            chain[i]->delete  = (void*)SAB_envelope_delete;
            break;
        case CUSTOM_FX:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_custom_fx_tst));
            chain[i]->init = (void*)SAB_custom_fx_init;
            chain[i]->process = (void*)SAB_custom_fx_process;
            chain[i]->delete  = (void*)SAB_custom_fx_delete;
            break;
        case PITCHDETECTOR:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_pitchdetector_tst));
            chain[i]->init = (void*)SAB_pitchdetector_init;
            chain[i]->process = (void*)SAB_pitchdetector_process;
            chain[i]->delete  = (void*)SAB_pitchdetector_delete;
            break;
        case SIG_GEN:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_generator_tst));
            chain[i]->init = (void*)SAB_generator_init;
            chain[i]->process = (void*)SAB_generator_process;
            chain[i]->delete  = (void*)SAB_generator_delete;
            break;
        case LPF:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_lpf_tst));
            chain[i]->init = (void*)SAB_lpf_init;
            chain[i]->process = (void*)SAB_lpf_process;
            chain[i]->delete  = (void*)SAB_lpf_delete;
            break;
        case HPF:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_hpf_tst));
            chain[i]->init = (void*)SAB_hpf_init;
            chain[i]->process = (void*)SAB_hpf_process;
            chain[i]->delete  = (void*)SAB_hpf_delete;
            break;
        case MARK2C:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_mark2c_tst));
            chain[i]->init = (void*)SAB_mark2c_init;
            chain[i]->process = (void*)SAB_mark2c_process;
            chain[i]->delete  = (void*)SAB_mark2c_delete;
            break;
        case IMRE:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_step_response_tst));
            chain[i]->init = (void*)SAB_step_response_init;
            chain[i]->process = (void*)SAB_step_response_process;
            chain[i]->delete  = (void*)SAB_step_response_delete;
            break;
        case HARMONIZER:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_harmonizer_tst));
            if (chain[i] != NULL) {
                chain[i]->init = (void*)SAB_harmonizer_init;
                chain[i]->process = (void*)SAB_harmonizer_process;
                chain[i]->delete = (void*)SAB_harmonizer_delete;
            }
            break;
        default:
            // TODO: ADD DUMMY DATA DEFINE HERE
             chain[i] = &DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN;
            break;
        }
        if(chain[i] == 0){
			chain[i] = &DEFAULT_EMPTY_SLOT_FOR_FX_CHAIN;
		}else{
	        // Initialize the effect
	        if(0 != chain[i]->init){
	            chain[i]->init(chain[i]);
	        }
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
    } else if (strcmp(fx_name_char, "Boost") == 0) {
        return BOOST;
    } else if (strcmp(fx_name_char, "Tremolo") == 0) {
        return TREMOLO;
    } else if (strcmp(fx_name_char, "Pitcshft") == 0) {
        return PITCHSHIFT;
    } else if (strcmp(fx_name_char, "Envelope") == 0) {
        return ENVELOPE;
    } else if (strcmp(fx_name_char, "Fuzz") == 0) {
		return FUZZ;
    } else if (strcmp(fx_name_char, "Reverb") == 0) {
		return REVERB;
    } else if (strcmp(fx_name_char, "Ptchdtcr") == 0) {
        return PITCHDETECTOR;
        } else if (strcmp(fx_name_char, "SIG_GEN") == 0) {
        return SIG_GEN;
    } else if (strcmp(fx_name_char, "lpf") == 0) {
        return LPF;
    } else if (strcmp(fx_name_char, "hpf") == 0) {
        return HPF;
    } else if (strcmp(fx_name_char, "bandpass") == 0) {
        return BANDPASS;
    } else if (strcmp(fx_name_char, "MarkIIc") == 0) {
        return MARK2C;
    } else if (strcmp(fx_name_char, "ImRe") == 0) {
        return IMRE;
    } else if (strcmp(fx_name_char, "Harmonizr") == 0) {
        return HARMONIZER;
    } else {
        
        // Handle unknown effect types
        // You might return a default value or handle it as an error
        // For example:
        return -1; // Invalid enum value
    }
}


// Function to process all effects in the chain
static inline void SAB_process_effect_chain(SAB_fx_manager_tst* self, GuitarEffect** chain, int chain_length) {
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
                    current_bypass->fx_states_aen[i] = slot_ptr[i].fx_state_en;
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
    clear_sdram_memory();
}

void SAB_fx_manager_deinit( SAB_fx_manager_tst* self){
    
}

#define USER_FLASH_ADDRESS 0x080E0000  // Base address of USER_FLASH section
#define NUM_OF_PRESETS 45
preset_saves_tst 	 SAB_PRESET_SAVE_FLASH_DATA[NUM_OF_PRESETS] __attribute__((section(".user_data")));
preset_saves_tst 	 SAB_PRESET_SAVE_RAM_DATA[NUM_OF_PRESETS] __attribute__((section(".preset_save_ram")));
#define SAB_PRESET_SAVE_STAGE_WORDS \
    (((sizeof(SAB_PRESET_SAVE_RAM_DATA) / 4U) + 7U) & ~7U)
static uint32_t SAB_PRESET_SAVE_STAGE[SAB_PRESET_SAVE_STAGE_WORDS]
    __attribute__((aligned(32)));
uint32_t SAB_preset_savefile_size(void){
    return (uint32_t)sizeof(SAB_PRESET_SAVE_FLASH_DATA);
}

uint8_t SAB_read_preset_savefile(uint32_t offset, uint8_t *dst, uint16_t length){
    uint32_t total_size = SAB_preset_savefile_size();
    if(dst == 0 || offset > total_size || (uint32_t)length > total_size - offset){
        return 0U;
    }
    memcpy(dst, ((const uint8_t *)SAB_PRESET_SAVE_FLASH_DATA) + offset, length);
    return 1U;
}

uint32_t SAB_save_preset_to_flash(SAB_fx_manager_tst* self){
    uint32_t preset_num_u32 =
        (self->intercom_pst->preset_data_un.preset_Major_u8 - 'A') * 9U +
        self->intercom_pst->preset_data_un.preset_Minor_u8 - 1U;
    fx_active_modes_ten current_mode = self->preset_mode_st.preset_mode_en;
    if(preset_num_u32 >= NUM_OF_PRESETS || current_mode >= NUM_OF_PRESET_MODES){
        return UINT32_MAX;
    }

    /* Snapshot the current live chain and this mode's state before committing. */
    fx_data_tst *slot_ptr = &self->intercom_pst->loop_data[0].slot1;
    preset_fx_bypasses_tst *current_bypass =
        &self->current_preset_config_st.bypass_states_st[current_mode];
    for(int i = 0; i < 12; i++){
        memcpy(self->current_preset_config_st.fx_names[i], slot_ptr[i].name,
               sizeof(self->current_preset_config_st.fx_names[i]));
        current_bypass->fx_states_aen[i] = slot_ptr[i].fx_state_en;
        for(int j = 0; j < 12; j++){
            self->current_preset_config_st.fx_params_value[i][j] =
                self->fx_instances[i]->intercom_parameters_aun[j].value_u8;
        }
    }
    memcpy(&current_bypass->loop_bypass_un,
           &self->intercom_pst->loopbypass_un, sizeof(sab_loopbypass_tun));

    memcpy(&SAB_PRESET_SAVE_RAM_DATA[preset_num_u32],
           &self->current_preset_config_st, sizeof(preset_saves_tst));
    memset(SAB_PRESET_SAVE_STAGE, 0xFF, sizeof(SAB_PRESET_SAVE_STAGE));
    memcpy(SAB_PRESET_SAVE_STAGE, SAB_PRESET_SAVE_RAM_DATA,
           sizeof(SAB_PRESET_SAVE_RAM_DATA));

    __disable_irq();
    uint32_t flash_save_err_u32 = Flash_Write_Data(
        (uint32_t)USER_FLASH_ADDRESS, SAB_PRESET_SAVE_STAGE,
        (uint16_t)SAB_PRESET_SAVE_STAGE_WORDS);
    __enable_irq();
    return flash_save_err_u32;
}

int32_t prev_preset_num_u32 = -1;
void inline SAB_load_preset_from_flash(SAB_fx_manager_tst* self){
    uint32_t preset_save_size_in_byte_u32 = 	sizeof(preset_saves_tst);
	uint32_t preset_save_size_in_word_u32 = 	preset_save_size_in_byte_u32 / 4;
	uint32_t preset_num_u32 = (self->intercom_pst->preset_data_un.preset_Major_u8-'A')*9+self->intercom_pst->preset_data_un.preset_Minor_u8-1;
	uint32_t save_location_addr_u32 = preset_save_size_in_byte_u32*preset_num_u32+USER_FLASH_ADDRESS;

  for(int i = 0; i<12;i++){
		// choose params
		for(int j=0;j<12;j++){
			self->current_preset_config_st.fx_params_value[i][j] = self->fx_instances[i]->intercom_parameters_aun[j].value_u8;
		}

	}

    if(prev_preset_num_u32 != -1){


        // save previous preset data
        memcpy(&SAB_PRESET_SAVE_RAM_DATA[prev_preset_num_u32],&self->current_preset_config_st,sizeof(preset_saves_tst));
    }
    prev_preset_num_u32 = preset_num_u32;

    // load the next from RAM
    memcpy(&self->current_preset_config_st,&SAB_PRESET_SAVE_RAM_DATA[preset_num_u32],sizeof(preset_saves_tst));
}


static inline  void SAB_load_current_config(SAB_fx_manager_tst* self ){
    for(int i=0; i<12;i++){
        self->fx_types_chain[i] = get_fx_type(self->current_preset_config_st.fx_names[i]);
    }



    init_effect_chain((GuitarEffect**)&self->fx_instances,self->fx_types_chain,12);

    for(int i = 0; i<12;i++){
         // choose params
         for(int j=0;j<12;j++){
        	 self->fx_instances[i]->intercom_parameters_aun[j].value_u8 = self->current_preset_config_st.fx_params_value[i][j];
         }
 	}
    // PARAMS ASSIGN / BYPASS SWITCHES SET

    fx_data_tst *slot_ptr= &self->intercom_pst->loop_data[0].slot1;
    for(int i=0; i<12;i++){
        if(self->fx_instances[i]->init != NULL){
            self->intercom_pst->fx_param_pun[i] = self->fx_instances[i]->intercom_parameters_aun;
            self->fx_instances[i]->intercom_fx_data.fx_state_en = self->current_preset_config_st.bypass_states_st[self->preset_mode_st.preset_mode_en].fx_states_aen[i];
			slot_ptr[i].fx_state_en 							= self->current_preset_config_st.bypass_states_st[self->preset_mode_st.preset_mode_en].fx_states_aen[i];
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
inline void  SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst, SAB_IO_HADRWARE_BUFFERS* hardware_IO_port_ptr, uint8_t* fsw1_ptr, uint8_t* fsw2_ptr){
    self->intercom_pst      		= intercom_pst;
    self->hardware_IO_port  		= hardware_IO_port_ptr;
    self->preset_mode_st.fsw1_ptr 	= fsw1_ptr;
    self->preset_mode_st.fsw2_ptr 	= fsw2_ptr;

    __disable_irq();  // Disable interrupts
    Flash_Read_Data((uint32_t)USER_FLASH_ADDRESS, (uint32_t*)&SAB_PRESET_SAVE_RAM_DATA, sizeof(preset_saves_tst)*NUM_OF_PRESETS/4);
    __enable_irq();   // Re-enable interrupts
    // LOAD DATA FROM FLASH
    SAB_load_preset_from_flash(self);
    SAB_load_current_config(self);
    
    if(self->current_preset_config_st.bypass_states_st[self->preset_mode_st.preset_mode_en].loop_bypass_un.L12!=0 && self->current_preset_config_st.bypass_states_st[self->preset_mode_st.preset_mode_en].loop_bypass_un.L12 != 1){
    	self->current_preset_config_st.bypass_states_st[self->preset_mode_st.preset_mode_en].loop_bypass_un.L12 = 1;
    }

    // Apply MODE A at boot: set the active mode, sync the display mirror
    // (fx_active_mode_u8), push this mode's fx states, AND copy the preset's
    // mode-A loop bypass into the intercom. Without this the intercom keeps
    // init_intercom's hardcoded default bypass (L12=1,L23=0,L34=1), the mode
    // mirror is never set, and the display/CDC report stale values.
    SAB_set_preset_mode(self, PRESET_MODE_A);
}

// ==================== FULL-STORE PRESET RESTORE (USB CDC) ====================
// The host stages a complete backup image in RAM (BEGIN + sequential CHUNKs);
// COMMIT validates the whole-image CRC-32 and only then erases + reprograms
// the preset sector. A corrupt or truncated backup never touches flash.

// Stage buffer rounded up to FLASHWORD (8 word = 32 byte) blocks so the final
// partial block of Flash_Write_Data never reads past the end of the buffer.
#define SAB_PRESET_RESTORE_STAGE_WORDS \
    (((sizeof(SAB_PRESET_SAVE_FLASH_DATA) / 4U) + 7U) & ~7U)

static uint8_t SAB_preset_restore_stage[SAB_PRESET_RESTORE_STAGE_WORDS * 4U]
    __attribute__((aligned(32)));

static struct {
    uint8_t  active_u8;
    uint32_t size_u32;
    uint32_t filled_u32;
    uint32_t expected_crc_u32;
    uint32_t running_crc_u32;   /* unfinalized value; init 0xFFFFFFFF */
} SAB_preset_restore_st;

// Incremental CRC-32 (IEEE, reflected, poly 0xEDB88320) - same family as the
// SABQ frame CRC (zlib.crc32): init 0xFFFFFFFF, final = state ^ 0xFFFFFFFF.
static uint32_t SAB_crc32_step(uint32_t state_u32, const uint8_t *data_p, uint32_t len_u32){
    while(len_u32-- > 0U){
        uint8_t byte_u8 = *data_p++;
        state_u32 ^= byte_u8;
        for(int i = 0; i < 8; i++){
            state_u32 = (state_u32 >> 1) ^ (0xEDB88320U * (uint32_t)(state_u32 & 1U));
        }
    }
    return state_u32;
}

uint8_t SAB_preset_restore_begin(uint32_t size_u32, uint32_t expected_crc_u32){
    if(size_u32 != SAB_preset_savefile_size()){
        return 0U;  // only a complete store image is accepted
    }
    memset(SAB_preset_restore_stage, 0xFF, sizeof(SAB_preset_restore_stage));
    SAB_preset_restore_st.active_u8        = 1U;
    SAB_preset_restore_st.size_u32         = size_u32;
    SAB_preset_restore_st.filled_u32       = 0U;
    SAB_preset_restore_st.expected_crc_u32 = expected_crc_u32;
    SAB_preset_restore_st.running_crc_u32  = 0xFFFFFFFFU;
    return 1U;
}

uint8_t SAB_preset_restore_chunk(uint32_t offset_u32, const uint8_t *data_p, uint16_t length_u16){
    if(!SAB_preset_restore_st.active_u8 || data_p == 0 || length_u16 == 0U){
        return 0U;
    }
    // Strictly sequential: the host sends contiguous chunks; out-of-order or
    // retried data must be rejected, not silently mis-staged.
    if(offset_u32 != SAB_preset_restore_st.filled_u32){
        return 0U;
    }
    if(offset_u32 + (uint32_t)length_u16 > SAB_preset_restore_st.size_u32){
        return 0U;
    }
    memcpy(&SAB_preset_restore_stage[offset_u32], data_p, length_u16);
    SAB_preset_restore_st.running_crc_u32 =
        SAB_crc32_step(SAB_preset_restore_st.running_crc_u32, data_p, (uint32_t)length_u16);
    SAB_preset_restore_st.filled_u32 = offset_u32 + (uint32_t)length_u16;
    return 1U;
}

uint8_t SAB_preset_restore_abort(void){
    SAB_preset_restore_st.active_u8 = 0U;
    return 1U;
}

uint8_t SAB_preset_restore_commit(SAB_fx_manager_tst* self){
    uint32_t final_crc_u32;
    uint32_t flash_err_u32;
    uint32_t preset_num_u32;

    if(self == 0 || !SAB_preset_restore_st.active_u8){
        return 0U;
    }
    if(SAB_preset_restore_st.filled_u32 != SAB_preset_restore_st.size_u32){
        return 0U;  // incomplete image -> nothing changes
    }
    final_crc_u32 = SAB_preset_restore_st.running_crc_u32 ^ 0xFFFFFFFFU;
    SAB_preset_restore_st.active_u8 = 0U;

    if(final_crc_u32 != SAB_preset_restore_st.expected_crc_u32){
        return 0U;  // CRC mismatch -> existing flash image preserved
    }

    // Auto-exit tuner first (it rebuilds from its pre-tuner snapshot), so the
    // stale snapshot cannot leak into the restored chain.
    if(self->tuner_active_u8){
        SAB_tuner_exit(self);
    }

    // CRC verified: erase the preset sector + reprogram from the staged image.
    // Same single-attempt discipline as SAB_save_preset_to_flash: IRQs off so a
    // failed write cannot spin here (only IWDG recovery is possible).
    __disable_irq();
    flash_err_u32 = Flash_Write_Data((uint32_t)USER_FLASH_ADDRESS,
                                     (uint32_t *)SAB_preset_restore_stage,
                                     (uint16_t)SAB_PRESET_RESTORE_STAGE_WORDS);
    if(flash_err_u32 == 0U){
        // Mirror the restored image into the RAM copy preset switching reads.
        memcpy(SAB_PRESET_SAVE_RAM_DATA, SAB_preset_restore_stage,
               SAB_preset_restore_st.size_u32);
    }
    __enable_irq();

    if(flash_err_u32 != 0U){
        return 0U;  // runtime RAM image untouched -> live preset unchanged
    }

    // Reload the current preset record straight from the restored RAM image
    // (NOT via SAB_load_preset_from_flash: its pre-save side effect would
    // write the pre-restore live state back over the fresh image first).
    preset_num_u32 = (uint32_t)((self->intercom_pst->preset_data_un.preset_Major_u8 - 'A') * 9
                             + self->intercom_pst->preset_data_un.preset_Minor_u8 - 1);
    if(preset_num_u32 < NUM_OF_PRESETS){
        memcpy(&self->current_preset_config_st,
               &SAB_PRESET_SAVE_RAM_DATA[preset_num_u32], sizeof(preset_saves_tst));
        prev_preset_num_u32 = (int32_t)preset_num_u32;
    }
    SAB_cleanup_effect_chain(self->fx_instances, 12);
    SAB_load_current_config(self);
    SAB_set_preset_mode(self, self->preset_mode_st.preset_mode_en);
    return 1U;
}



// ==================== TUNER MODE (FSW2+FSW3) ====================
// Entry: place the pitch detector in Loop1 slot 0, forced ON, saving the
// original slot-0 effect so it can be restored exactly on exit.
inline void SAB_tuner_enter(SAB_fx_manager_tst* self){
    if(self->tuner_active_u8) return;
    fx_active_modes_ten mode = self->preset_mode_st.preset_mode_en;

    // Save ALL 12 slot names so the user's chain is restored exactly on exit.
    for(int i = 0; i < 12; ++i){
        memcpy(self->tuner_saved_st.names[i], self->current_preset_config_st.fx_names[i], 10);
    }
    self->tuner_saved_st.state0_u8 = self->current_preset_config_st.bypass_states_st[mode].fx_states_aen[0];

    // Place the pitch detector in slot 0, forced ON. Clear every other slot so its
    // buffers are NOT allocated from the SDRAM pool - this frees room for the large
    // (25000-sample) detection buffer the tuner needs to be accurate.
    memcpy(self->current_preset_config_st.fx_names[0], "Ptchdtcr", sizeof("Ptchdtcr"));
    self->current_preset_config_st.fx_names[0][9] = 0;
    self->current_preset_config_st.bypass_states_st[mode].fx_states_aen[0] = FX_STATE_ON;
    for(int i = 1; i < 12; ++i){
        self->current_preset_config_st.fx_names[i][0] = 0;   // empty name -> no effect instantiated
    }

    // Rebuild the chain: cleanup() resets the SDRAM bump allocator (clear_sdram_memory),
    // then load allocates ONLY the pitch detector's buffers into the fresh pool.
    SAB_cleanup_effect_chain(self->fx_instances,12);
    SAB_load_current_config(self);

    // mark tuner active (main.c reads this to drive FSW_LED2/3)
    self->tuner_active_u8 = 1;
    if(self->intercom_pst) self->intercom_pst->tuner_data_un.active_u8 = 1;
}

// Exit: restore Loop1 slot 0 exactly as it was before entry, then rebuild.
inline void SAB_tuner_exit(SAB_fx_manager_tst* self){
    if(!self->tuner_active_u8) return;
    fx_active_modes_ten mode = self->preset_mode_st.preset_mode_en;

    // Restore ALL 12 slot names exactly as they were before entry, then bring the
    // user's full chain back: cleanup() resets SDRAM, load re-allocates every effect.
    for(int i = 0; i < 12; ++i){
        memcpy(self->current_preset_config_st.fx_names[i], self->tuner_saved_st.names[i], 10);
    }
    self->current_preset_config_st.bypass_states_st[mode].fx_states_aen[0] = self->tuner_saved_st.state0_u8;

    SAB_cleanup_effect_chain(self->fx_instances,12);
    SAB_load_current_config(self);

    // mark tuner inactive (main.c clears FSW_LED2/3)
    self->tuner_active_u8 = 0;
    if(self->intercom_pst) self->intercom_pst->tuner_data_un.active_u8 = 0;
}

// Toggle tuner mode (FSW2+FSW3 chord).
inline void SAB_tuner_toggle(SAB_fx_manager_tst* self){
    if(self->tuner_active_u8) SAB_tuner_exit(self);
    else                      SAB_tuner_enter(self);
}


// Apply a preset mode: set it, sync the I2C mirror for the display, and push
// the per-mode fx states + loop bypasses to the intercom.
inline void  SAB_set_preset_mode(SAB_fx_manager_tst* self, fx_active_modes_ten mode_en){
    // auto-exit tuner mode before changing modes (restores Loop1 slot 0)
    if(self->tuner_active_u8) SAB_tuner_exit(self);

    if(mode_en >= NUM_OF_PRESET_MODES){
        mode_en = PRESET_MODE_A;
    }
    self->preset_mode_st.preset_mode_en = mode_en;

    // keep the I2C mirror of the active mode in sync for the display
    self->intercom_pst->preset_data_un.fx_active_mode_u8 = mode_en;

    fx_data_tst *slot_ptr= &self->intercom_pst->loop_data[0].slot1;
    // update fx states
    for(int i=0; i<12;i++){
        self->fx_instances[i]->intercom_fx_data.fx_state_en = self->current_preset_config_st.bypass_states_st[mode_en].fx_states_aen[i];
        slot_ptr[i].fx_state_en                             = self->current_preset_config_st.bypass_states_st[mode_en].fx_states_aen[i];
    }
    // update loops
    memcpy(&self->intercom_pst->loopbypass_un,&self->current_preset_config_st.bypass_states_st[mode_en].loop_bypass_un,sizeof(sab_loopbypass_tun));
}

inline void  SAB_preset_up_pressed(SAB_fx_manager_tst* self){
    // auto-exit tuner mode before changing presets (restores Loop1 slot 0)
    if(self->tuner_active_u8) SAB_tuner_exit(self);
    fx_active_modes_ten mode_en = self->preset_mode_st.preset_mode_en;

    if (self->intercom_pst->preset_data_un.preset_Minor_u8 == 9 && self->intercom_pst->preset_data_un.preset_Major_u8 == 'Z')
	{
	}
	else
	{
		self->intercom_pst->preset_data_un.preset_Minor_u8++;
		if (self->intercom_pst->preset_data_un.preset_Minor_u8 > 9)
		{
			if (self->intercom_pst->preset_data_un.preset_Major_u8 < 'D')
			{
                self->intercom_pst->preset_data_un.preset_Minor_u8 = 1;
				self->intercom_pst->preset_data_un.preset_Major_u8++;
			}else{
                self->intercom_pst->preset_data_un.preset_Minor_u8 = 9;
            }
		}
        // LOAD PRESET DATA FROM FLASH
        SAB_load_preset_from_flash(self);
        // DELETE CURRENT PRESET
        SAB_cleanup_effect_chain((GuitarEffect**)self,12);
        // INIT NEW PRESET
        SAB_load_current_config(self);
        SAB_set_preset_mode(self, mode_en);
        HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
		HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);


	}
}
inline void  SAB_preset_down_pressed(SAB_fx_manager_tst* self){
    // auto-exit tuner mode before changing presets (restores Loop1 slot 0)
    if(self->tuner_active_u8) SAB_tuner_exit(self);
    fx_active_modes_ten mode_en = self->preset_mode_st.preset_mode_en;

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
        SAB_cleanup_effect_chain((GuitarEffect**)self,12);
        // INIT NEW PRESET
        SAB_load_current_config(self);
        SAB_set_preset_mode(self, mode_en);
        HAL_GPIO_WritePin(FSW_LED1_GPIO_Port, FSW_LED1_Pin, 0);
		HAL_GPIO_WritePin(FSW_LED2_GPIO_Port, FSW_LED2_Pin, 0);
	}
}

inline void SAB_fx_manager_process( SAB_fx_manager_tst* self){
    if(self->intercom_pst->change_occured_flg != 0){
        if(SAB_handle_intercom_change(self)){
            for(int i = 0; i<12;i++){
                // choose params
                for(int j=0;j<12;j++){
                    self->current_preset_config_st.fx_params_value[i][j] = self->fx_instances[i]->intercom_parameters_aun[j].value_u8;
                }
            }
            SAB_cleanup_effect_chain(self->fx_instances,12);
            SAB_load_current_config(self);
        }
    }
    SAB_process_effect_chain(self,self->fx_instances,12);

    // TUNER DATA -> display (publish pitch detector output + active flag)
    if(self->intercom_pst){
        float32_t tuner_freq_f32 = 0.0f;
        if(self->tuner_active_u8 && self->fx_instances[0] != NULL
           && self->fx_types_chain[0] == PITCHDETECTOR){
            tuner_freq_f32 = ((SAB_pitchdetector_tst*)self->fx_instances[0])->detected_freq_f32;
        }
        self->intercom_pst->tuner_data_un.freq_f32   = tuner_freq_f32;
        self->intercom_pst->tuner_data_un.active_u8  = self->tuner_active_u8;
    }
}
