/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.10. */

#ifndef SAB_OVERDRIVE_H
#define SAB_OVERDRIVE_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: overdrive
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
        float32_t gain_f32;
        float32_t tone_f32;
        float32_t volume_f32;
} SAB_overdrive_tst;


// Process Function for SAB_overdrive_tst
void SAB_overdrive_init( SAB_overdrive_tst* self, float input_f32, float output_f32){};

// Process Function for SAB_overdrive_tst
int SAB_overdrive_process( SAB_overdrive_tst* self, float input_f32, float output_f32){};



#endif