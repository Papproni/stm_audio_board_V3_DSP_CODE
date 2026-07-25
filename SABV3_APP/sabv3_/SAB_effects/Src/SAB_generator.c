/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on:  */

#include "SAB_generator.h"
#include <math.h>
#include <stdint.h>


void SAB_generator_delete( SAB_generator_tst* self){
    // TODO
}

// Process Function for SAB_generator_tst
void SAB_generator_init( SAB_generator_tst* self){

    strcpy(self->intercom_fx_data.name, "SIG_GEN");
	self->intercom_fx_data.color[0] = 10; 	// R
	self->intercom_fx_data.color[1] = 50;	// G
	self->intercom_fx_data.color[2] = 50;	// B
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
// Process Function for SAB_generator_tst
float32_t SAB_generator_process( SAB_generator_tst* self, float32_t input_f32){
    // Freq
    self->param_1_value = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,50, 8000);
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
