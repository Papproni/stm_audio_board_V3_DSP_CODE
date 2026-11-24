/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_BOOST_H
#define SAB_BOOST_H

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
} SAB_boost_tst;


// Process Function for SAB_boost_tst
void SAB_boost_init( SAB_boost_tst* self);

// Process Function for SAB_boost_tst
float32_t SAB_boost_process( SAB_boost_tst* self, float input_f32);

// Process Function for SAB_boost_tst
void SAB_boost_delete( SAB_boost_tst* self);


#endif