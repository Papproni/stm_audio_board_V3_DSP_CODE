/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_delay.h"

static void calculate_buffer_location(SAB_delay_tst* self){
	// current_pointer + buffer[time]
	self->delayed_counter_u32 = self->current_counter_u32 + self->time_in_buffer_u32;

	// if delay sample is bigger than the buffer go back
	if(self->delayed_counter_u32 > (SAB_DELAY_BUFFER_SIZE-1) ){
		self->delayed_counter_u32 = self->delayed_counter_u32- (SAB_DELAY_BUFFER_SIZE-1);
	}
}

static float32_t get_delayed_signal(SAB_delay_tst* self){
	return self->data_samples[self->delayed_counter_u32];
}

static void increment_current_sample_counter(SAB_delay_tst* self){
	self->current_counter_u32++;
	if(self->current_counter_u32 > (SAB_DELAY_BUFFER_SIZE-1) ){
		self->current_counter_u32 = 0;
	}
}



void SAB_delay_delete( SAB_delay_tst* self){
    // TODO
}


// Process Function for SAB_delay_tst
void SAB_delay_init( SAB_delay_tst* self){
    strcpy(self->intercom_fx_data.name, "Delay");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 255;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_ON;

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

	self->current_counter_u32  =0;
    self->delayed_counter_u32 = 0;
    self->time_in_buffer_u32 = 0;
	self->data_samples  = sdram_malloc_float32_t_array(SAB_DELAY_BUFFER_SIZE);
    for(int i = 0; i< SAB_DELAY_BUFFER_SIZE;i++){
    	self->data_samples[i] = 0;
    }
};

// Process Function for SAB_delay_tst
float32_t SAB_delay_process( SAB_delay_tst* self, float input_f32){
	self->feedback_f32  			= conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 1);
	self->time_in_buffer_u32 		= conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,1, SAB_DELAY_BUFFER_SIZE-1);
	self->mix_f32 					= conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 1);
	
	calculate_buffer_location(self);

	// delay effect
	float32_t delayed_sample_f32 = get_delayed_signal(self);
    float32_t output_f32 = (float)delayed_sample_f32*self->mix_f32 +
						input_f32*(1.0 - self->mix_f32);
    increment_current_sample_counter(self);

	// feedback calculation input,delayed,feedback
	self->data_samples[self->current_counter_u32] = 	( (float)delayed_sample_f32* self->feedback_f32) +
													(input_f32 * (1.0 - self->feedback_f32));
    return output_f32;
};

