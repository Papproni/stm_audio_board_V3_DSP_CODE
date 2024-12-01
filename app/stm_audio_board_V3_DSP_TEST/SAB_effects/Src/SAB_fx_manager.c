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
void process_effect_chain(GuitarEffect** chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        chain[i]->process(chain[i]);
    }
}

// Cleanup the effects in the chain
void cleanup_effect_chain(GuitarEffect** chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        chain[i]->delete(chain[i]);  // Free the allocated memory for each effect
        free(chain[i]);
    }
}

void SAB_fx_manager_deinit( SAB_fx_manager_tst* self){
    // TODO
}




// FLASH MEMORY
//  __attribute__((section(".user_data"))) preset_saves_tst presets_flash_st[26*9];

#define USER_FLASH_ADDRESS 0x08080000  // Base address of USER_FLASH section

void SAB_save_preset_to_flash(SAB_fx_manager_tst* self){
	preset_saves_tst preset_flash_st;
	// 1. Calculate data addr
	uint32_t preset_save_size_in_byte_u32 = 	sizeof(preset_saves_tst);
	uint32_t preset_save_size_in_word_u32 = 	preset_save_size_in_byte_u32 / 4;
	uint32_t preset_num_u32 = (self->intercom_pst->preset_data_un.preset_Major_u8-'A')*9+self->intercom_pst->preset_data_un.preset_Minor_u8-1;
	uint32_t save_location_addr_u32 = preset_save_size_in_byte_u32*preset_num_u32+USER_FLASH_ADDRESS;
	// COPY:
	// 2.1. loop data
    for(int i=0;i<NUM_OF_LOOPS;i++){
        memcpy(&preset_flash_st.loop_data[i].slot1,&self->fx_instances[i*NUM_OF_FX_SLOTS_IN_LOOP+i]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&preset_flash_st.loop_data[i].slot1,&self->fx_instances[i*NUM_OF_FX_SLOTS_IN_LOOP+i+1]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&preset_flash_st.loop_data[i].slot1,&self->fx_instances[i*NUM_OF_FX_SLOTS_IN_LOOP+i+2]->intercom_fx_data,sizeof(fx_data_tst));
    }
	// 2.2. fx params
	for(int i = 0; i<(NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS);i++){
			memcpy(preset_flash_st.fx_params_tun[i],self->fx_instances[i]->intercom_parameters_aun,sizeof(sab_fx_param_tun)*NUM_OF_MAX_PARAMS);
	}
	// 2.3. loopbypass
	// memcpy(preset_flash_st.loopbypass_un.all_u8,self->loopbypass_un.all_u8,sizeof(sab_loopbypass_tun));


	// 3. save to flash
	Flash_Write_Data(save_location_addr_u32,&preset_flash_st,preset_save_size_in_word_u32);
}

void SAB_load_preset_from_flash(SAB_fx_manager_tst* self){

}

void SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst){
    // Load data from saved files
    // for(int i=0; i<4; i++){
    //     self->fx_chain_names[i*3+0] = intercom_pst->loop_data[i].slot1.name;
    //     self->fx_chain_names[i*3+1] = intercom_pst->loop_data[i].slot2.name;
    //     self->fx_chain_names[i*3+2] = intercom_pst->loop_data[i].slot3.name;
    // }

    self->intercom_pst = intercom_pst;


    strcpy(self->fx_chain_names[0],"Octave");
    strcpy(self->fx_chain_names[1], "Delay");
    strcpy(self->fx_chain_names[2], "CUSTOM_FX");
    strcpy(self->fx_chain_names[3],"Flanger");
    strcpy(self->fx_chain_names[4], "Chorus");
    strcpy(self->fx_chain_names[5], "Overdrive");
    strcpy(self->fx_chain_names[6], "Distortion");
    for(int i=0; i<12;i++){
        // MUST ADD THIS VARIABLE FIRST FROM FLASH!!!!
        self->fx_types_chain[i] = get_fx_type(self->fx_chain_names[i]);
    }

    init_effect_chain(&self->fx_instances,self->fx_types_chain,12);

    // LOOP DATA
    
   // Display data assign
    for(int i=0; i<4;i++){
        memcpy(&intercom_pst->loop_data[i].slot1, &self->fx_instances[i*3+0]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&intercom_pst->loop_data[i].slot2, &self->fx_instances[i*3+1]->intercom_fx_data,sizeof(fx_data_tst));
        memcpy(&intercom_pst->loop_data[i].slot3, &self->fx_instances[i*3+2]->intercom_fx_data,sizeof(fx_data_tst));
    }

    
 // PARAMS ASSIGN
    for(int i=0; i<12;i++){
        intercom_pst->fx_param_pun[i] = self->fx_instances[i]->intercom_parameters_aun;
    }
    // Switch mode set to normal


}


