/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_BOOST_H
#define SAB_BOOST_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: boost
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t boost_f32;
} SAB_boost_tst;


// Process Function for SAB_boost_tst
void SAB_boost_init( SAB_boost_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_boost_tst
int SAB_boost_process( SAB_boost_tst* self, float input_f32, float output_f32){};



#endif