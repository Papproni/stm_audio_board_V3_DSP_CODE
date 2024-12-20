/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_ENVELOPE_FILTER_H
#define SAB_ENVELOPE_FILTER_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: boost
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
    void (*delete)(void*);
     
    fx_data_tst			intercom_fx_data;
	sab_fx_param_tun 	intercom_parameters_aun[NUM_OF_MAX_PARAMS];


    arm_biquad_cascade_df2T_instance_f32 biquad_filter;
    float32_t biquad_filter_coeffs_af32[5];
    float32_t biquad_filter_states_af32[4];
    float32_t biquad_filter_output_f32;

    float32_t param_1_value;
    float32_t param_2_value;
    float32_t param_3_value;
    float32_t param_4_value;
    
    float32_t vol_f32;
    float32_t drv_f32;
    float32_t Q_f32;
    float32_t atk_time_f32;
    float32_t release_time_f32;

    float32_t envelope_f32;
    float32_t freq_f32;
    float32_t cutoff_f32;

} SAB_envelope_tst;


// Process Function for SAB_boost_tst
void SAB_envelope_init( SAB_envelope_tst* self);

// Process Function for SAB_boost_tst
float32_t SAB_envelope_process( SAB_envelope_tst* self, float input_f32);

// Process Function for SAB_boost_tst
void SAB_envelope_delete( SAB_envelope_tst* self);


#endif
