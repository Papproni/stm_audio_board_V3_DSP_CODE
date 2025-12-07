/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_MARK2C_H
#define SAB_MARK2C_H

#include "sab_intercom.h"
#include "stdint.h"
#define float32_t float

// Effect: overdrive
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

    float32_t gain_pre_f32   ;
    float32_t treb_db_f32    ;
    float32_t mid_db_f32     ;
    float32_t bass_db_f32    ;
    float32_t lead_drive_f32 ;
    float32_t geq_80_db_f32  ;
    float32_t geq_240_db_f32 ;
    float32_t geq_750_db_f32 ;
    float32_t geq_2200_db_f32;
    float32_t geq_6600_db_f32;
    float32_t presence_db_f32;
    float32_t master_f32     ;

    // -----------------
    // 1) INPUT HPF (~90 Hz)
    // -----------------
    float32_t hpf_a_f32;
    float32_t hpf_x1_f32;
    float32_t hpf_y1_f32;

    // -----------------
    // 2) TONE STACK (3-band, pre-distortion)
    // -----------------
    arm_biquad_cascade_df2T_instance_f32 tonestack;
    float32_t tonestack_coeffs_af32[5 * 3]; // 3 stages
    float32_t tonestack_state_af32[4 * 3];

    uint8_t last_treb_raw_u8;
    uint8_t last_mid_raw_u8;
    uint8_t last_bass_raw_u8;

    // -----------------
    // 3) GEQ (5-band, post distortion)
    // -----------------
    arm_biquad_cascade_df2T_instance_f32 geq;
    float32_t geq_coeffs_af32[5 * 5];  // 5 stages
    float32_t geq_state_af32[4 * 5];
    uint8_t last_geq_raw_u8[5];        // 80, 240, 750, 2200, 6600

    // -----------------
    // 4) PRESENCE shelf
    // -----------------
    arm_biquad_cascade_df2T_instance_f32 presence;
    float32_t presence_coeffs_af32[5]; // 1 stage
    float32_t presence_state_af32[4];
    uint8_t last_presence_raw_u8;

    // -----------------
    // 5) CATHODE FOLLOWER softening LPF
    // -----------------
    float32_t cf_a_f32;      // LPF coefficient
    float32_t cf_y1_f32;     // last output

} SAB_mark2c_tst;


// Process Function for SAB_mark2c_tst
void SAB_mark2c_init( SAB_mark2c_tst* self);

// Process Function for SAB_mark2c_tst
float32_t SAB_mark2c_process( SAB_mark2c_tst* self, float input_f32);

// Process Function for SAB_mark2c_tst
void SAB_mark2c_delete( SAB_mark2c_tst* self);


#endif