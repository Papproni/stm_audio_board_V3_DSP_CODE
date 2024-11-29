/*
 * SAB_fx_manager.c
 *
 *  Created on: Nov 10, 2024
 *      Author: pappr
 */

#include "SAB_fx_manager.h"


void init_effect_chain(GuitarEffect** chain, EffectType* fx_chain, int chain_length) {
    for (int i = 0; i < chain_length; ++i) {
        switch (fx_chain[i]) {
        case DELAY:
            chain[i] = (GuitarEffect*)malloc(sizeof(SAB_delay_tst));
            chain[i]->init = SAB_delay_init;
            chain[i]->process = SAB_delay_process;
            chain[i]->delete  = SAB_delay_delete;
            break;
        // case OVERDRIVE:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_overdrive_tst));
        //     chain[i]->init = SAB_overdrive_init;
        //     chain[i]->process = SAB_overdrive_process;
        //     chain[i]->delete  = SAB_overdrive_delete;
        //     break;
        // case DISTORTION:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_distortion_tst));
        //     chain[i]->init = SAB_distortion_init;
        //     chain[i]->process = SAB_distortion_process;
        //     chain[i]->delete  = SAB_distortion_delete;
        //     break;
        case OCTAVE:
            chain[i] = (GuitarEffect*)malloc(sizeof(octave_effects_tst));
            chain[i]->init = SAB_octave_init;
            chain[i]->process = SAB_octave_process;
            chain[i]->delete  = SAB_octave_delete;
            break;
        // case FLANGER:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_flanger_tst));
        //     chain[i]->init = SAB_flanger_init;
        //     chain[i]->process = SAB_flanger_process;
        //     chain[i]->delete  = SAB_flanger_delete;
        //     break;
        // case CHORUS:
        //     chain[i] = (GuitarEffect*)malloc(sizeof(SAB_chorus_tst));
        //     chain[i]->init = SAB_chorus_init;
        //     chain[i]->process = SAB_chorus_process;
        //     chain[i]->delete  = SAB_chorus_delete;
        //     break;
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
            printf("Unknown effect type!\n");
            break;
        }
        // Initialize the effect
        chain[i]->init(chain[i]);
    }
}

EffectType get_fx_type(char* fx_name_char) {
    if (strcmp(fx_name_char, "Delay") == 0) {
        return DELAY;
    } else if (strcmp(fx_name_char, "Octave") == 0) {
        return OCTAVE;
    } else if (strcmp(fx_name_char, "CUSTOM_FX") == 0) {
        return CUSTOM_FX;
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
    }
}

void SAB_fx_manager_deinit( SAB_fx_manager_tst* self){
    // TODO
}

void SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst){
    for(int i=0; i<4; i++){
        self->fx_chain_names[i*3+0] = intercom_pst->loop_data[i].slot1.name;
        self->fx_chain_names[i*3+1] = intercom_pst->loop_data[i].slot2.name;
        self->fx_chain_names[i*3+2] = intercom_pst->loop_data[i].slot3.name;
    }
    for(int i=0; i<12;i++){
        self->fx_types_chain[i] = get_fx_type(self->fx_chain_names[i]);
    }

    init_effect_chain(&self->fx_instances,self->fx_types_chain,12);
}


