/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_flanger.h"
#include "arm_math.h"

static void calculate_buffer_location(SAB_flanger_tst* self){
	// current_pointer - buffer[time] (subtract for flanger style)
	self->delayed_counter_i32 = self->current_counter_i32 - (int32_t)self->depth_f32;
    
    if (self->delayed_counter_i32 < 0){
        self->delayed_counter_i32 = (SAB_FLANGER_BUFFER_SIZE - 1) + self->delayed_counter_i32;
    }
	// if delay sample is bigger than the buffer go back
	if(self->delayed_counter_i32 > (SAB_FLANGER_BUFFER_SIZE - 1)){
		self->delayed_counter_i32 = self->delayed_counter_i32 - (SAB_FLANGER_BUFFER_SIZE - 1);
	}
}

static float32_t get_delayed_signal(SAB_flanger_tst* self){
	return self->data_samples[self->delayed_counter_i32];
}

static void increment_current_sample_counter(SAB_flanger_tst* self){
	self->current_counter_i32++;
	if(self->current_counter_i32 > (SAB_FLANGER_BUFFER_SIZE - 1)){
		self->current_counter_i32 = 0;
	}
}


void SAB_flanger_delete( SAB_flanger_tst* self){
    // TODO
}


// Process Function for SAB_flanger_tst
void SAB_flanger_init( SAB_flanger_tst* self){
    strcpy(self->intercom_fx_data.name, "Flanger");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    add_parameter(&self->intercom_parameters_aun[0],"RATE",PARAM_TYPE_POT,120);  // Default sweep rate ~500
    add_parameter(&self->intercom_parameters_aun[1],"MAXD",PARAM_TYPE_POT,200);  // Default max delay ~70
    add_parameter(&self->intercom_parameters_aun[2],"MIND",PARAM_TYPE_POT,30);   // Default min delay ~2
    add_parameter(&self->intercom_parameters_aun[3],"MIX",PARAM_TYPE_POT,255);  // Default mix = 1.0 (fully wet)
    add_parameter(&self->intercom_parameters_aun[4],"FB",PARAM_TYPE_POT,130);   // Default feedback ~0.5
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    self->current_counter_i32  = 0;
    self->delayed_counter_i32 = 0;  // Will be set by calculate_buffer_location
    self->modulation_counter_f32 = 1.0;  // Direction: 1 = up, -1 = down
    self->depth_f32 = 20.0;  // Start with delay in middle of typical range
    self->param_1_value = 50.0;  // Sweep counter starts at 50 for faster initial response
    
    // Initialize buffer
    for(int i = 0; i < SAB_FLANGER_BUFFER_SIZE; i++){
    	self->data_samples[i] = 0;
    }
};

// Process Function for SAB_flanger_tst
float32_t SAB_flanger_process( SAB_flanger_tst* self, float input_f32){
    // Read parameters from potentiometers (fixed ranges)
    float32_t sweep_rate = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8, 50, 500);   // Sweep rate: 50-500 samples between changes (faster sweep with higher pot)
    float32_t max_delay = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8, 10, 70);    // Max delay: 10-70 samples (longer delay with higher pot)
    float32_t min_delay = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8, 2, 20);     // Min delay: 2-20 samples (longer delay with higher pot)
    self->mix_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8, 0.3, 1.0);        // Wet/dry mix: 0.3-1.0 (more wet with higher pot)
    self->feedback_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8, 0.1, 0.7);   // Feedback: 0.1-0.7 (more feedback with higher pot)
    
    // Triangle wave sweep (like Flanger_sweep in the example)
    self->param_1_value -= 1.0;  // Decrement sweep counter
    if (self->param_1_value <= 0.0) {  
        self->depth_f32 += self->modulation_counter_f32 * 2.0;  // Step by 2.0 samples for faster modulation
        
        if (self->depth_f32 > max_delay) {
            self->modulation_counter_f32 = -1.0;  // Reverse to go down
        } 
        if (self->depth_f32 < min_delay) {
            self->modulation_counter_f32 = 1.0;   // Reverse to go up
        }
        
        self->param_1_value = sweep_rate * 0.5;  // Reset sweep counter (faster updates)
    }
    
    // Ensure delay time stays within bounds
    if(self->depth_f32 < 2) self->depth_f32 = 2;
    if(self->depth_f32 > SAB_FLANGER_BUFFER_SIZE - 2) self->depth_f32 = SAB_FLANGER_BUFFER_SIZE - 2;
    
    // Calculate read position
    calculate_buffer_location(self);
    
    // Get the delayed sample
    float32_t delayed_sample_f32 = get_delayed_signal(self);
    
    // Flanger output: mix delayed with input (like the delay effect)
    float32_t output_f32 = delayed_sample_f32 * self->mix_f32 + input_f32 * (1.0 - self->mix_f32);
    
    // Write to buffer with feedback (like the delay effect)
    self->data_samples[self->current_counter_i32] = (delayed_sample_f32 * self->feedback_f32) + 
                                                     (input_f32 * (1.0 - self->feedback_f32));
    
    // Increment write counter
    increment_current_sample_counter(self);
    
    return output_f32;
};


