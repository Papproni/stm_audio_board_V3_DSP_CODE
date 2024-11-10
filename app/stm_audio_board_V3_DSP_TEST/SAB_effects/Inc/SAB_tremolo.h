/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_TREMOLO_H
#define SAB_TREMOLO_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: tremolo
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t mix_f32;
        float32_t rate_f32;
        float32_t depth_f32;
} SAB_tremolo_tst;


// Process Function for SAB_tremolo_tst
void SAB_tremolo_init( SAB_tremolo_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_tremolo_tst
int SAB_tremolo_process( SAB_tremolo_tst* self, float input_f32, float output_f32){};



#endif