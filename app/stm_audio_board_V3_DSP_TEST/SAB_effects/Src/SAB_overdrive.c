/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_overdrive.h"

void SAB_overdrive_delete( SAB_overdrive_tst* self){
    // TODO
}

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

static void calculate_biquad_coeffs(float32_t* coeffs_af32,FilterType type, float freq, float Q, float sample_rate) {
    float omega = 2.0f * 3.14159265358979323846 * freq / sample_rate;
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


// Process Function for SAB_overdrive_tst
void SAB_overdrive_init( SAB_overdrive_tst* self){
    strcpy(self->intercom_fx_data.name, "Overdrive");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 255;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"GAIN",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"TONE",PARAM_TYPE_POT,69 );
    add_parameter(&self->intercom_parameters_aun[2],"LVL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"GAIN",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    arm_biquad_cascade_df2T_init_f32(&self->biquad_filter, 1, &self->biquad_filter_coeffs_af32, &self->biquad_filter_states_af32);
    self->biquad_filter_output_f32  = 0;


    self->ths_f32 = INT32_MAX/5/4;
};

static float32_t limit(float32_t in, float32_t min, float32_t max){
	if (in>max){
		in = max;
	}else if(in<-min){
		in = -min;
	}
	return in;
}

// Process Function for SAB_overdrive_tst
float32_t SAB_overdrive_process( SAB_overdrive_tst* self, float input_f32){
    self->gain_f32 		= conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 10);
	self->tone_f32		= conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,700, 1800);
    self->volume_f32    = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 4);

    float32_t Q_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.2, 3);
    float32_t output_f32;
    // Clip
    output_f32 = limit(input_f32*self->gain_f32, self->ths_f32,self->ths_f32);
    // LPF
    calculate_biquad_coeffs(&self->biquad_filter_coeffs_af32,LPF, self->tone_f32, Q_f32, 48000);
    arm_biquad_cascade_df2T_f32(&self->biquad_filter, &output_f32, &self->biquad_filter_output_f32, 1);
    output_f32 = (self->biquad_filter_output_f32)*self->volume_f32;
    return output_f32;
};

