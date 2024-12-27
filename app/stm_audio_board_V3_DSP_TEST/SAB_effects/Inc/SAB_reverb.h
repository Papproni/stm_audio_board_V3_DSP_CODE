/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_REVERB_H
#define SAB_REVERB_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float


#define l_CB0 3460*2
#define l_CB1 2988*2
#define l_CB2 3882*2
#define l_CB3 4312*2
#define l_AP0 480*2
#define l_AP1 161*2
#define l_AP2 46*2



// Effect: reverb
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

    float32_t wet_f32;
    float32_t time_f32;

    //define pointer limits = delay time
    int cf0_lim, cf1_lim, cf2_lim, cf3_lim, ap0_lim, ap1_lim, ap2_lim;
    float32_t cfbuf0[l_CB0], cfbuf1[l_CB1], cfbuf2[l_CB2], cfbuf3[l_CB3];
    float32_t apbuf0[l_AP0], apbuf1[l_AP1], apbuf2[l_AP2];
    //feedback defines as of Schroeder
    float32_t cf0_g, cf1_g, cf2_g, cf3_g;
    float32_t ap0_g, ap1_g, ap2_g;
    //buffer-pointer
    uint32_t cf0_p, cf1_p, cf2_p, cf3_p, ap0_p, ap1_p, ap2_p;


    
} SAB_reverb_tst;


// Process Function for SAB_delay_tst
void SAB_reverb_init( SAB_reverb_tst* self);

// Process Function for SAB_delay_tst
float32_t SAB_reverb_process( SAB_reverb_tst* self, float input_f32);

// Process Function for SAB_delay_tst
void SAB_reverb_delete( SAB_reverb_tst* self);


#endif
