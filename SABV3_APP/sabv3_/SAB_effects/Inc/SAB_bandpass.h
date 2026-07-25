/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on:  */

#ifndef SAB_BANDPASS_H
#define SAB_BANDPASS_H

#include "sab_intercom.h"
#include "stdint.h"
#include "arm_math.h"
#define float32_t float

// Effect: bandpass
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
    void (*delete)(void*);
     
    fx_data_tst			intercom_fx_data;
	sab_fx_param_tun 	intercom_parameters_aun[NUM_OF_MAX_PARAMS];

    
    float32_t param_1_value;
    float32_t param_2_value;
    float32_t param_3_value;
    float32_t param_4_value;
    float32_t param_5_value;
    float32_t param_6_value;
    float32_t param_7_value;
    float32_t param_8_value;
    float32_t param_9_value;
    float32_t param_10_value;
    float32_t param_11_value;
    float32_t param_12_value;

    // Generated outputs from other Jinja templates
    
    float32_t input_block_block0_output_f32;
    
    // Biquad filter header for biquad_filter_block_block2
arm_biquad_cascade_df2T_instance_f32 biquad_filter_block_block2;
float32_t biquad_filter_block_block2_coeffs_af32[5];
float32_t biquad_filter_block_block2_states_af32[4];
float32_t biquad_filter_block_block2_output_f32;
    
    float32_t output_block_block1_input_f32;
    

} SAB_bandpass_tst;


// Process Function for SAB_bandpass_tst
void SAB_bandpass_init( SAB_bandpass_tst* self);

// Process Function for SAB_bandpass_tst
float32_t SAB_bandpass_process( SAB_bandpass_tst* self, float32_t input_f32);

void SAB_bandpass_delete( SAB_bandpass_tst* self);

#endif