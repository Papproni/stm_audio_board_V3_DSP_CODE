/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_equalizer.h"

void SAB_equalizer_delete( SAB_equalizer_tst* self){
    // TODO
}


// Process Function for SAB_equalizer_tst
void SAB_equalizer_init( SAB_equalizer_tst* self){
    strcpy(self->intercom_fx_data.name, "equalizer");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"GAIN",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_POT,69);
};

// Process Function for SAB_equalizer_tst
float32_t SAB_equalizer_process( SAB_equalizer_tst* self, float input_f32){
    // TODO
};

