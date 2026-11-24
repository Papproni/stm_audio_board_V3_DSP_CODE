/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on:  */

#include "SAB_bandpass.h"
#include "SAB_biquad.h"
#include <math.h>
#include <stdint.h>





void SAB_bandpass_delete( SAB_bandpass_tst* self){
    // TODO
}

// Process Function for SAB_bandpass_tst
void SAB_bandpass_init( SAB_bandpass_tst* self){

    strcpy(self->intercom_fx_data.name, "bandpass");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"FREQ",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"Q",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"GAIN",PARAM_TYPE_POT,69);
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
    /*
// Biquad filter init
Type: bandpass
Freq: param_1
Q:    param_2
*/

arm_biquad_cascade_df2T_init_f32(&self->biquad_filter_block_block2, 1, (float32_t*)&self->biquad_filter_block_block2_coeffs_af32,  (float32_t*)&self->biquad_filter_block_block2_states_af32);
self->biquad_filter_block_block2_output_f32  = 0;

};

// Process Function for SAB_bandpass_tst
float32_t SAB_bandpass_process( SAB_bandpass_tst* self, float32_t input_f32){
    // Freq
    self->param_1_value = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,50, 10000);
    // Q
    self->param_2_value = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.1, 5);
    // Gain
    self->param_3_value = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0.001, 5);
    // Generated outputs from other Jinja templates
    
    self->input_block_block0_output_f32 = input_f32;
    
    // Biquad filter process function
SAB_biquad_coeffs_legacy(self->biquad_filter_block_block2_coeffs_af32, SAB_BIQUAD_BANDPASS, 48000.0f, self->param_1_value, self->param_2_value);
arm_biquad_cascade_df2T_f32(&self->biquad_filter_block_block2, &self->input_block_block0_output_f32, &self->biquad_filter_block_block2_output_f32, 1);
self->biquad_filter_block_block2_output_f32 *= self->param_3_value;
    
    // output_block_block1
self->output_block_block1_input_f32 = self->biquad_filter_block_block2_output_f32;
return self->output_block_block1_input_f32;
    
};