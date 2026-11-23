/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_OVERDRIVE_H
#define SAB_OVERDRIVE_H

#include "sab_intercom.h"
#include "SAB_biquad.h"
#include "stdint.h"
#define float32_t float

#define SAB_PI 3.14159265359f


// Effect: overdrive
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


    //----------------------------------------------------------------
    SAB_biquad_tst input_lpf_16k;
    SAB_biquad_tst tight_hpf;
    SAB_biquad_tst pre_lpf;

    SAB_biquad_tst stage1_lpf;
    SAB_biquad_tst inter_hpf;
    SAB_biquad_tst inter_mid;
    SAB_biquad_tst stage2_lpf;

    SAB_biquad_tst bass;
    SAB_biquad_tst mid;
    SAB_biquad_tst treble;
    SAB_biquad_tst presence;

    SAB_biquad_tst cab_hpf;
    SAB_biquad_tst cab_lpf;
    SAB_biquad_tst fizz_notch;

    float env;
    float fs;

    float gain;
    float tight;
    float bass_knob;
    float mid_knob;
    float treble_knob;
    float presence_knob;
    float volume;

        float32_t gain_f32;
        float32_t tone_f32;
        float32_t volume_f32;

        float32_t ths_f32;

    arm_biquad_cascade_df2T_instance_f32 biquad_filter;
    float32_t biquad_filter_coeffs_af32[5];
    float32_t biquad_filter_states_af32[4];
    float32_t biquad_filter_output_f32;
} SAB_overdrive_tst;


// Process Function for SAB_overdrive_tst
void SAB_overdrive_init( SAB_overdrive_tst* self);

// Process Function for SAB_overdrive_tst
float32_t SAB_overdrive_process( SAB_overdrive_tst* self, float input_f32);

// Process Function for SAB_overdrive_tst
void SAB_overdrive_delete( SAB_overdrive_tst* self);


#endif