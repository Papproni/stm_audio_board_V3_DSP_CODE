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
    
    add_parameter(&self->intercom_parameters_aun[0],"FREQ",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"AMP",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[3],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    // Generated outputs from other Jinja templates
    self->generator_block_block1_offs = 0;
self->generator_block_block1_time = 0;
    self->phase_f32 = 0;
};

float32_t max_amplitude= 3e+009;
// Process Function for SAB_custom_fx_tst
float32_t SAB_custom_fx_process( SAB_custom_fx_tst* self, float32_t input_f32){
    // Freq
    self->param_1_value = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,70, 5000);
    // Amp
    self->param_2_value = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0, max_amplitude);

    // Generated outputs from other Jinja templates

    self->generator_block_block1_phase += self->param_1_value * (1.0f / 48000.0f);

    // wrap into [0,1) without jumps
    if (self->generator_block_block1_phase >= 1.0f)
        self->generator_block_block1_phase -= 1.0f;

// generator_block_block1 process: SINE
    // self->generator_block_block1_phase = fmod(self->generator_block_block1_time, 1.0/self->param_1_value) * self->param_1_value;

    self->generator_block_block1_output_f32 = self->param_2_value * sin(2 * 3.141592653 * self->generator_block_block1_phase);

    self->generator_block_block1_output_f32 += self->generator_block_block1_offs;
    
    // output_block_block2
self->output_block_block2_input_f32 = self->generator_block_block1_output_f32;
return self->output_block_block2_input_f32;
    
};
