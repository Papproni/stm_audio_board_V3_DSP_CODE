/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_FX_MANAGER_H
#define SAB_FX_MANAGER_H


#include "stdint.h"
#include "sab_intercom.h"
#include "main.h"

// Generated effect libs
#include "SAB_delay.h"
#include "SAB_overdrive.h"
#include "SAB_distortion.h"
//#include "SAB_octave.h"
#include "SAB_flanger.h"
#include "SAB_chorus.h"
#include "SAB_boost.h"
#include "SAB_pitchshift.h"
#include "SAB_fuzz.h"
#include "SAB_tremolo.h"
#include "SAB_equalizer.h"
#include "SAB_CUSTOM.h"
#include "SAB_envelope_filter.h"
#include "SAB_reverb.h"
#include "SAB_custom_fx.h"
#include "SAB_octave.h"

#include "stdint.h"
#include "arm_math.h"


#include "FLASH_SECTOR_H7.h"


// Enum representing different effect types
typedef enum {
    REVERB,
    DELAY,
    OVERDRIVE,
    DISTORTION,
    OCTAVE,
    FLANGER,
    CHORUS,
    BOOST,
    PITCHSHIFT,
    FUZZ,
    TREMOLO,
    EQUALIZER,
    CUSTOM,
    CUSTOM_FX,
    ENVELOPE
} EffectType;

volatile typedef struct{
	int32_t in1_i32;
	int32_t in2_i32;
	int32_t in3_i32;
	int32_t in4_i32;

	int32_t out1_i32;
	int32_t out2_i32;
	int32_t out3_i32;
	int32_t out4_i32;
}SAB_IO_HADRWARE_BUFFERS;

typedef struct preset_bypasses_st{
	sab_loopbypass_tun 	loop_bypass_un;
	fx_state_ten 	    fx_states_aen[NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS];
}preset_fx_bypasses_tst;

#define NUM_OF_PRESET_MODES 3
typedef enum {
	PRESET_MODE_NORMAL,
	PRESET_MODE_A_ACTIVE,
	PRESET_MODE_B_ACTIVE
}fx_active_modes_ten;

typedef struct preset_saves_st{
    // Store FX names used in preset
	char 	                fx_names[NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS][10];
    // Store their parameters setting
	uint8_t 	            fx_params_value[NUM_OF_FX_SLOTS_IN_LOOP*NUM_OF_LOOPS][NUM_OF_MAX_PARAMS];
    preset_fx_bypasses_tst	bypass_states_st[NUM_OF_PRESET_MODES];
}preset_saves_tst; 


// Struct for a guitar effect with function pointers
typedef struct {
    void (*init)(void*);
    float32_t (*process)(void*,float32_t input);
    void (*delete)(void*);
    fx_data_tst			intercom_fx_data;
	sab_fx_param_tun 	intercom_parameters_aun[NUM_OF_MAX_PARAMS];
} GuitarEffect;

typedef struct{
    uint8_t *fsw1_ptr;
    uint8_t *fsw2_ptr;
    fx_active_modes_ten preset_mode_en;
}SAB_fsw_mode_tst;


// Manager structure: fx_manager 
typedef struct {
	GuitarEffect*   fx_instances[12];
    EffectType      fx_types_chain[12];
    char          fx_chain_names[12][10];

    SAB_fsw_mode_tst        preset_mode_st;
    SAB_IO_HADRWARE_BUFFERS* hardware_IO_port;

    sab_intercom_tst *intercom_pst;
    
    preset_saves_tst current_preset_config_st;
} SAB_fx_manager_tst;




// INIT for SAB_fx_manager_tst
void SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst, SAB_IO_HADRWARE_BUFFERS* hardware_IO_port_ptr, uint8_t* fsw1_ptr, uint8_t* fsw2_ptr);

// INIT for SAB_fx_manager_tst
void SAB_fx_manager_deinit( SAB_fx_manager_tst* self);

// Process Function for SAB_fx_manager_tst
void SAB_fx_manager_process( SAB_fx_manager_tst* self);

void init_effect_chain(GuitarEffect** chain, EffectType* fx_chain, int chain_length);

void process_effect_chain(GuitarEffect** chain, int chain_length);

void cleanup_effect_chain(GuitarEffect** chain, int chain_length);

void SAB_save_preset_to_flash(SAB_fx_manager_tst* self);
void SAB_load_preset_from_flash(SAB_fx_manager_tst* self);

void SAB_fsw_pressed_callback(SAB_fx_manager_tst* self);
void SAB_preset_up_pressed(SAB_fx_manager_tst* self);
void SAB_preset_down_pressed(SAB_fx_manager_tst* self);
#endif
