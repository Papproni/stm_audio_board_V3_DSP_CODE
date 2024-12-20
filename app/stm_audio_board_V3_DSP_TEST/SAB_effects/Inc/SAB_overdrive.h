/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_OVERDRIVE_H
#define SAB_OVERDRIVE_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

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
        float32_t gain_f32;
        float32_t tone_f32;
        float32_t volume_f32;
} SAB_overdrive_tst;


// Process Function for SAB_overdrive_tst
void SAB_overdrive_init( SAB_overdrive_tst* self);

// Process Function for SAB_overdrive_tst
float32_t SAB_overdrive_process( SAB_overdrive_tst* self, float input_f32);

// Process Function for SAB_overdrive_tst
void SAB_overdrive_delete( SAB_overdrive_tst* self);


#endif