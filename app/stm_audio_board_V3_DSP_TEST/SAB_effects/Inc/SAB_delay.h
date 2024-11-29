/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_DELAY_H
#define SAB_DELAY_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: delay
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
        float32_t mix_f32;
        float32_t time_f32;
        float32_t feedback_f32;
} SAB_delay_tst;


// Process Function for SAB_delay_tst
void SAB_delay_init( SAB_delay_tst* self);

// Process Function for SAB_delay_tst
float32_t SAB_delay_process( SAB_delay_tst* self, float input_f32);

// Process Function for SAB_delay_tst
void SAB_delay_delete( SAB_delay_tst* self);


#endif