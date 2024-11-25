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

static float32_t conv_raw_to_param_value(uint8_t value, float32_t low, float32_t high) {
	float32_t v_f32 = value;
    return low + (v_f32 / 255.0f) * (high - low);
}

// Process Function for SAB_custom_fx_tst
void SAB_custom_fx_init( SAB_custom_fx_tst* self){

    strcpy(self->intercom_fx_data.name, "CUSTOM_FX");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"FREQ",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"Q",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"GAIN",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_POT,69);

    // Generated outputs from other Jinja templates
    /*
// Biquad filter init
Type: BANDPASS
Freq: param_1
Q:    param_2
*/

arm_biquad_cascade_df2T_init_f32(&self->BIQUAD_FILTER_block5, 1, &self->BIQUAD_FILTER_block5_coeffs_af32, &self->BIQUAD_FILTER_block5_states_af32);
self->BIQUAD_FILTER_block5_output_f32  = 0;

};

// Process Function for SAB_custom_fx_tst
float32_t SAB_custom_fx_process( SAB_custom_fx_tst* self, float32_t input_f32, float32_t output_f32){
    // Freq
    self->param_1_value = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,20, 10000);
    // Q
    self->param_2_value = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.1, 10);
    // Gain
    self->param_3_value = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 10);

    // Generated outputs from other Jinja templates
    
    self->INPUT_BLOCK_block1_output_f32 = input_f32;
    
    // Biquad filter process function
calculate_biquad_coeffs(self->BIQUAD_FILTER_block5_coeffs_af32,BANDPASS, self->param_1_value, self->param_2_value, 48000);
arm_biquad_cascade_df2T_f32(&self->BIQUAD_FILTER_block5, &self->INPUT_BLOCK_block1_output_f32, &self->BIQUAD_FILTER_block5_output_f32, 1);
self->BIQUAD_FILTER_block5_output_f32 *= self->param_3_value;

    self->ADD_BLOCK_block6_output_f32 = self->INPUT_BLOCK_block1_output_f32 + self->BIQUAD_FILTER_block5_output_f32;
    
    self->OUTPUT_BLOCK_block3_input_f32 = self->ADD_BLOCK_block6_output_f32;
output_f32 = self->OUTPUT_BLOCK_block3_input_f32;
    

    return output_f32;
};
