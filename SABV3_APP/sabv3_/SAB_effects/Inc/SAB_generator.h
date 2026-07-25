#ifndef SAB_GENERATOR_H
#define SAB_GENERATOR_H

#include "sab_intercom.h"
#include "stdint.h"
#include "arm_math.h"
#define float32_t float

// Effect: generator
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
    
    // Generator header for generator_block_block1
float32_t generator_block_block1_freq;
float32_t generator_block_block1_amp;
float32_t generator_block_block1_offs;
float32_t generator_block_block1_time;
float32_t generator_block_block1_phase;
float32_t generator_block_block1_output_f32;
    
    float32_t output_block_block2_input_f32;
    
    float32_t phase_f32;

} SAB_generator_tst;


// Process Function for SAB_generator_tst
void SAB_generator_init( SAB_generator_tst* self);

// Process Function for SAB_generator_tst
float32_t SAB_generator_process( SAB_generator_tst* self, float32_t input_f32);

void SAB_generator_delete( SAB_generator_tst* self);

#endif