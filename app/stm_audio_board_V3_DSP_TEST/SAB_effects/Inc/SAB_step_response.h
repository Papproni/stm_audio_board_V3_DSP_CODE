/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_STEP_RESPONSE_H
#define SAB_STEP_RESPONSE_H

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

    float32_t param_1_value;
    float32_t boost_f32;
    
    // float32_t  step_response_data_af32[10000];
} SAB_step_response_tst;


// Process Function for SAB_step_response_tst
void SAB_step_response_init( SAB_step_response_tst* self);

// Process Function for SAB_step_response_tst
float32_t SAB_step_response_process( SAB_step_response_tst* self, float input_f32);

// Process Function for SAB_step_response_tst
void SAB_step_response_delete( SAB_step_response_tst* self);


#endif