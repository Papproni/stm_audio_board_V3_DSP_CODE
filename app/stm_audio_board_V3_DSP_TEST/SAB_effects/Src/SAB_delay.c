/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_delay.h"

void SAB_delay_delete( SAB_delay_tst* self){
    // TODO
}


// Process Function for SAB_delay_tst
void SAB_delay_init( SAB_delay_tst* self){
    strcpy(self->intercom_fx_data.name, "Delay");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 255;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
	add_parameter(&self->intercom_parameters_aun[0],"MIX",	PARAM_TYPE_POT,	69);
	add_parameter(&self->intercom_parameters_aun[1],"TIME",	PARAM_TYPE_POT,	120);
	add_parameter(&self->intercom_parameters_aun[2],"FBK",	PARAM_TYPE_POT,	85);
	add_parameter(&self->intercom_parameters_aun[3],"MOD",	PARAM_TYPE_BTN,	0);
	add_parameter(&self->intercom_parameters_aun[4],"BASE",	PARAM_TYPE_POT,	10);
	add_parameter(&self->intercom_parameters_aun[5],"AMP",	PARAM_TYPE_UNUSED,	10);
    add_parameter(&self->intercom_parameters_aun[6],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

};

// Process Function for SAB_delay_tst
float32_t SAB_delay_process( SAB_delay_tst* self, float input_f32){
    // TODO
};
