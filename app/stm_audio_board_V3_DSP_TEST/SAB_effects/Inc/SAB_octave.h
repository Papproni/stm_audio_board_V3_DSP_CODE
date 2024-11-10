/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_OCTAVE_H
#define SAB_OCTAVE_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: octave
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t octave1_up_vol_f32;
        float32_t octave2_up_vol_f32;
        float32_t octave1_down_vol_f32;
        float32_t octave2_down_vol_f32;
        float32_t normal_vol_f32;
} SAB_octave_tst;


// Process Function for SAB_octave_tst
void SAB_octave_init( SAB_octave_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_octave_tst
int SAB_octave_process( SAB_octave_tst* self, float input_f32, float output_f32){};



#endif