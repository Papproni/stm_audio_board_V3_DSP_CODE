/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on:  */

#include "SAB_custom_fx.h"
#include <math.h>
#include <stdint.h>



typedef enum {
    LPF,
    HPF,
    BANDPASS,
    NOTCH
} FilterType;

typedef struct {
    float b0, b1, b2; // Numerator coefficients
    float a1, a2;     // Denominator coefficients
} BiquadCoeffs;

void calculate_biquad_coeffs(float32_t* coeffs_af32,FilterType type, float freq, float Q, float sample_rate) {
    float omega = 2.0f * 3.14 * freq / sample_rate;
    float alpha = sinf(omega) / (2.0f * Q);
    float cos_omega = cosf(omega);

    BiquadCoeffs coeffs;

    switch (type) {
        case LPF:
            coeffs.b0 = (1.0f - cos_omega) / 2.0f;
            coeffs.b1 = 1.0f - cos_omega;
            coeffs.b2 = (1.0f - cos_omega) / 2.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case HPF:
            coeffs.b0 = (1.0f + cos_omega) / 2.0f;
            coeffs.b1 = -(1.0f + cos_omega);
            coeffs.b2 = (1.0f + cos_omega) / 2.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case BANDPASS:
            coeffs.b0 = alpha;
            coeffs.b1 = 0.0f;
            coeffs.b2 = -alpha;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case NOTCH:
            coeffs.b0 = 1.0f;
            coeffs.b1 = -2.0f * cos_omega;
            coeffs.b2 = 1.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;
    }

    // Normalize coefficients
    float a0 = 1.0f + alpha;
    coeffs.b0 /= a0;
    coeffs.b1 /= a0;
    coeffs.b2 /= a0;
    coeffs.a1 /= a0;
    coeffs.a2 /= a0;

    coeffs_af32[0]=coeffs.b0;
    coeffs_af32[1]=coeffs.b1;
    coeffs_af32[2]=coeffs.b2;
    coeffs_af32[3]=-coeffs.a1;
    coeffs_af32[4]=-coeffs.a2;
}

void SAB_custom_fx_delete( SAB_custom_fx_tst* self){
    // TODO
}

// Process Function for SAB_custom_fx_tst
void SAB_custom_fx_init( SAB_custom_fx_tst* self){

    strcpy(self->intercom_fx_data.name, "CUSTOM_FX");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"FREQ1",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"Q1",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"VOL1",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"FREQ2",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"Q2",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"VOL2",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    // Generated outputs from other Jinja templates
    /*
// Biquad filter init
Type: BANDPASS
Freq: param_1
Q:    param_2
*/

arm_biquad_cascade_df2T_init_f32(&self->biquad_filter_block_block2, 1, &self->biquad_filter_block_block2_coeffs_af32, &self->biquad_filter_block_block2_states_af32);
self->biquad_filter_block_block2_output_f32  = 0;/*
// Biquad filter init
Type: BANDPASS
Freq: param_4
Q:    param_5
*/

arm_biquad_cascade_df2T_init_f32(&self->biquad_filter_block_block3, 1, &self->biquad_filter_block_block3_coeffs_af32, &self->biquad_filter_block_block3_states_af32);
self->biquad_filter_block_block3_output_f32  = 0;

};

// Process Function for SAB_custom_fx_tst
float32_t SAB_custom_fx_process( SAB_custom_fx_tst* self, float32_t input_f32){
    // Freq1
    self->param_1_value = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,100, 800);
    // Q1
    self->param_2_value = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.1, 5);
    // Vol1
    self->param_3_value = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 5);
    // Freq2
    self->param_4_value = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,3000, 6000);
    // Q2
    self->param_5_value = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8,0.1, 5);
    // Vol2
    self->param_6_value = conv_raw_to_param_value(self->intercom_parameters_aun[5].value_u8,0, 5);

    // Generated outputs from other Jinja templates
    
    self->input_block_block1_output_f32 = input_f32;
    
    // Biquad filter process function
calculate_biquad_coeffs(self->biquad_filter_block_block2_coeffs_af32,BANDPASS, self->param_1_value, self->param_2_value, 48000);
arm_biquad_cascade_df2T_f32(&self->biquad_filter_block_block2, &self->input_block_block1_output_f32, &self->biquad_filter_block_block2_output_f32, 1);
self->biquad_filter_block_block2_output_f32 *= self->param_3_value;
    
    // Biquad filter process function
calculate_biquad_coeffs(self->biquad_filter_block_block3_coeffs_af32,BANDPASS, self->param_4_value, self->param_5_value, 48000);
arm_biquad_cascade_df2T_f32(&self->biquad_filter_block_block3, &self->input_block_block1_output_f32, &self->biquad_filter_block_block3_output_f32, 1);
self->biquad_filter_block_block3_output_f32 *= self->param_6_value;
    
    // add_block_block4
self->add_block_block4_output_f32 = self->biquad_filter_block_block2_output_f32 + self->biquad_filter_block_block3_output_f32;
    
    // output_block_block0
self->output_block_block0_input_f32 = self->add_block_block4_output_f32;
return self->output_block_block0_input_f32;
    
};