/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_PITCHSHIFT_H
#define SAB_PITCHSHIFT_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: pitchshift
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t pitch_f32;
        float32_t mix_f32;
        float32_t vol_f32;
} SAB_pitchshift_tst;


// Process Function for SAB_pitchshift_tst
void SAB_pitchshift_init( SAB_pitchshift_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_pitchshift_tst
int SAB_pitchshift_process( SAB_pitchshift_tst* self, float input_f32, float output_f32){};



#endif