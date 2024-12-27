/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_reverb.h"



void SAB_reverb_delete( SAB_reverb_tst* self){
    // TODO
}


// Process Function for SAB_delay_tst
void SAB_reverb_init( SAB_reverb_tst* self){
    strcpy(self->intercom_fx_data.name, "Delay");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 255;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
	add_parameter(&self->intercom_parameters_aun[0],"FBK",	PARAM_TYPE_POT,	69);
	add_parameter(&self->intercom_parameters_aun[1],"TIME",	PARAM_TYPE_POT,	120);
	add_parameter(&self->intercom_parameters_aun[2],"MIX",	PARAM_TYPE_POT,	85);
	add_parameter(&self->intercom_parameters_aun[3],"NONE",	PARAM_TYPE_UNUSED,	0);
	add_parameter(&self->intercom_parameters_aun[4],"NONE",	PARAM_TYPE_UNUSED,	10);
	add_parameter(&self->intercom_parameters_aun[5],"NONE",	PARAM_TYPE_UNUSED,	10);
    add_parameter(&self->intercom_parameters_aun[6],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);


};

// Process Function for SAB_delay_tst
float32_t SAB_reverb_process( SAB_reverb_tst* self, float input_f32){

	float32_t output_f32;
    return output_f32;
};

