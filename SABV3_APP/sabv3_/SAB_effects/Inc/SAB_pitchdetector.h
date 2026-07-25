/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_PITCHDETECTOR_H
#define SAB_PITCHDETECTOR_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

#ifndef PITCH_DETECTOR_BUFFER_SIZE
#define PITCH_DETECTOR_BUFFER_SIZE 25000
#endif
// Effect: pitchdetector
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
	
    float32_t input_raw_af32[PITCH_DETECTOR_BUFFER_SIZE];
    float32_t centered_f32[PITCH_DETECTOR_BUFFER_SIZE];
    // float32_t input_fltrd_af32[PITCH_DETECTOR_BUFFER_SIZE];
    // float32_t autocorr_af32[PITCH_DETECTOR_BUFFER_SIZE][2];

    float32_t average_f32;
    int input_raw_cntr_u32;
	float32_t detected_freq_f32;
    uint8_t   sign_bits[PITCH_DETECTOR_BUFFER_SIZE];


} SAB_pitchdetector_tst;


// Process Function for SAB_pitchdetector_tst
void SAB_pitchdetector_init( SAB_pitchdetector_tst* self);

// Process Function for SAB_pitchdetector_tst
float32_t SAB_pitchdetector_process( SAB_pitchdetector_tst* self, float input_f32);

// Process Function for SAB_pitchdetector_tst
void SAB_pitchdetector_delete( SAB_pitchdetector_tst* self);


#endif