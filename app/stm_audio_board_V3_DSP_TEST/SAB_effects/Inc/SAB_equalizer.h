/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_EQUALIZER_H
#define SAB_EQUALIZER_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: equalizer
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t vol_f32;
        float32_t gain_f32;
        float32_t 20hz_f32;
        float32_t 40hz_f32;
        float32_t 80hz_f32;
        float32_t 160hz_f32;
        float32_t 315hz_f32;
        float32_t 630hz_f32;
        float32_t 1.25khz_f32;
        float32_t 2.5khz_f32;
        float32_t 5khz_f32;
        float32_t 10khz_f32;
        float32_t 20khz_f32;
} SAB_equalizer_tst;


// Process Function for SAB_equalizer_tst
void SAB_equalizer_init( SAB_equalizer_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_equalizer_tst
int SAB_equalizer_process( SAB_equalizer_tst* self, float input_f32, float output_f32){};



#endif