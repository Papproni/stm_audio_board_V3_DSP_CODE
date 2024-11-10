/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_CHORUS_H
#define SAB_CHORUS_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: chorus
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t level_f32;
        float32_t rate_f32;
        float32_t filter_f32;
        float32_t depth_f32;
} SAB_chorus_tst;


// Process Function for SAB_chorus_tst
void SAB_chorus_init( SAB_chorus_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_chorus_tst
int SAB_chorus_process( SAB_chorus_tst* self, float input_f32, float output_f32){};



#endif