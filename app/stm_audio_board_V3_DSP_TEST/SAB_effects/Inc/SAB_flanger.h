/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_FLANGER_H
#define SAB_FLANGER_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: flanger
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t rate_f32;
        float32_t depth_f32;
        float32_t manual_f32;
        float32_t res_f32;
} SAB_flanger_tst;


// Process Function for SAB_flanger_tst
void SAB_flanger_init( SAB_flanger_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_flanger_tst
int SAB_flanger_process( SAB_flanger_tst* self, float input_f32, float output_f32){};



#endif