/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_CHORUS_H
#define SAB_CHORUS_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: chorus

#define SAB_CHORUS_BUFFER_SIZE 5000

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
        float32_t level_f32;
        float32_t rate_f32;
        float32_t mix_f32;
        float32_t depth_f32;
        float32_t feedback_f32;
    float32_t data_samples[SAB_CHORUS_BUFFER_SIZE];

    uint32_t delayed_counter_u32;
    uint32_t current_counter_u32;
    uint32_t time_in_buffer_u32;
    uint32_t modulation_counter_u32;
} SAB_chorus_tst;


// Process Function for SAB_chorus_tst
void SAB_chorus_init( SAB_chorus_tst* self);

// Process Function for SAB_chorus_tst
float32_t SAB_chorus_process( SAB_chorus_tst* self, float input_f32);

// Process Function for SAB_chorus_tst
void SAB_chorus_delete( SAB_chorus_tst* self);


#endif
