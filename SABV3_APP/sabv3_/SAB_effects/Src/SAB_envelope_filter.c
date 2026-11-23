/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_envelope_filter.h"
#include "SAB_biquad.h"
#include <math.h> // Required for the log() function





void SAB_envelope_delete( SAB_envelope_tst* self){
    // TODO
}


// Process Function for SAB_boost_tst
void SAB_envelope_init( SAB_envelope_tst* self){
    strcpy(self->intercom_fx_data.name, "Envelope");
	self->intercom_fx_data.color[0] = 231; 	// R
	self->intercom_fx_data.color[1] = 111;	// G
	self->intercom_fx_data.color[2] = 117;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    add_parameter(&self->intercom_parameters_aun[0],"VOL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"Q",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"THS",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"ATK",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"RLS",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"GA",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[6],"GR",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    // Filter init
    arm_biquad_cascade_df2T_init_f32(&self->biquad_filter, 1, (float32_t*)&self->biquad_filter_coeffs_af32,  (float32_t*)&self->biquad_filter_states_af32);
    self->biquad_filter_output_f32  = 0;

    self->atk_time_f32 = 100;
    self->release_time_f32 = 100;
};

// Process Function for SAB_boost_tst
float32_t SAB_envelope_process( SAB_envelope_tst* self, float input_f32){
    self->vol_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0,5);
    self->Q_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.01,20);
    float32_t threshold = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0.1,3);
    float32_t ATK = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,100,400);
    float32_t RLS = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8,100,400);
    float32_t GA = conv_raw_to_param_value(self->intercom_parameters_aun[5].value_u8,100,1000);
    float32_t GR = conv_raw_to_param_value(self->intercom_parameters_aun[6].value_u8,100,1000);

    // self->Q_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,0,5);
    
    // Calcualte freq
    self->freq_f32 = 0.0;

    float32_t rectified = fabs(input_f32);


    float32_t magic_nominal_gain = 43171048;
    float32_t delta = (rectified / magic_nominal_gain);

    if (rectified > 43171048*threshold) {
        // Attack phase
    	self->cutoff_f32  += GA * (1.0f - expf(-1.0f / (ATK)));
    } else {
        // Release phase
    	self->cutoff_f32  -= GR * (1.0f - expf(-1.0f / (RLS)));
    }

    if (self->cutoff_f32 < 200.0f) self->cutoff_f32 = 200.0f;                  // Set minimum cutoff frequency
    if (self->cutoff_f32 > 3000.0f) self->cutoff_f32 = 3000.0f;              // Set maximum cutoff frequency

    SAB_biquad_coeffs_legacy(self->biquad_filter_coeffs_af32, SAB_BIQUAD_LPF, 48000.0f, self->cutoff_f32, self->Q_f32);
    arm_biquad_cascade_df2T_f32(&self->biquad_filter, &input_f32, &self->biquad_filter_output_f32, 1);

    return self->biquad_filter_output_f32*self->vol_f32;
};

