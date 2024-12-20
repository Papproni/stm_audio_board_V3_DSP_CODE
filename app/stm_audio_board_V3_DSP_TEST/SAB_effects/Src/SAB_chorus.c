/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_chorus.h"

static void calculate_buffer_location(SAB_chorus_tst* self){
	// current_pointer + buffer[time]
	self->delayed_counter_u32 = self->current_counter_u32 + self->time_in_buffer_u32;

	// if delay sample is bigger than the buffer go back
	if(self->delayed_counter_u32 > (SAB_CHORUS_BUFFER_SIZE-1) ){
		self->delayed_counter_u32 = self->delayed_counter_u32- (SAB_CHORUS_BUFFER_SIZE-1);
	}
}

static float32_t get_delayed_signal(SAB_chorus_tst* self){
	return self->data_samples[self->delayed_counter_u32];
}

static void increment_current_sample_counter(SAB_chorus_tst* self){
	self->current_counter_u32++;
	if(self->current_counter_u32 > (SAB_CHORUS_BUFFER_SIZE-1) ){
		self->current_counter_u32 = 0;
	}
}


void SAB_chorus_delete( SAB_chorus_tst* self){
    // TODO
}


// Process Function for SAB_chorus_tst
void SAB_chorus_init( SAB_chorus_tst* self){
    strcpy(self->intercom_fx_data.name, "Chorus");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 100;	// G
	self->intercom_fx_data.color[2] = 50;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;
    

	// PARAMS:
    add_parameter(&self->intercom_parameters_aun[0],"LvL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"RATE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"MIX",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"DPH",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"FBK",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    self->current_counter_u32  =0;
    self->delayed_counter_u32 = 0;
    self->time_in_buffer_u32 = 0;

    for(int i = 0; i< SAB_CHORUS_BUFFER_SIZE;i++){
    	self->data_samples[i] = 0;
    }
};

// Process Function for SAB_chorus_tst
float32_t SAB_chorus_process( SAB_chorus_tst* self, float input_f32){
     self->level_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 8);
     self->rate_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,96000, 10000);
     self->mix_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 1);
     self->depth_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,5, 100);
     self->feedback_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8,1, 0);
    calculate_buffer_location(self);

	// delay effect
	float32_t delayed_sample_f32 = get_delayed_signal(self);
    float32_t output_f32 = (float)delayed_sample_f32*self->mix_f32 +
						input_f32*(1.0 - self->mix_f32);
    increment_current_sample_counter(self);

    // modulation
    self->time_in_buffer_u32 = 4800 + self->depth_f32*sin((float)self->modulation_counter_u32/(float)self->rate_f32*6.28);
    self->modulation_counter_u32++;

	// feedback calculation input,delayed,feedback
	self->data_samples[self->current_counter_u32] = 	( (float)delayed_sample_f32* self->feedback_f32) +
													(input_f32 * (1.0 - self->feedback_f32));
    return output_f32*self->level_f32;
};

