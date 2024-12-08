/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_tremolo.h"
#include "arm_math.h"
void SAB_tremolo_delete( SAB_tremolo_tst* self){
    // TODO
}


// Process Function for SAB_tremolo_tst
void SAB_tremolo_init( SAB_tremolo_tst* self){
    strcpy(self->intercom_fx_data.name, "Tremolo");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 255;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"SPD",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"DPH",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"TRI",PARAM_TYPE_BTN,69);
    add_parameter(&self->intercom_parameters_aun[3],"VOL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    self->phase = 0;
};


static float generate_triangle_wave(float phase) {
    if (phase < 0.5f) {
        // Rising edge
        return 2.0f * phase;
    } else {
        // Falling edge
        return 2.0f * (1.0f - phase);
    }
}

// Process Function for SAB_tremolo_tst
float32_t SAB_tremolo_process( SAB_tremolo_tst* self, float input_f32){
    float32_t output_f32;

    self->rate_f32          = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0.5, 10);
    self->depth_f32         = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,1, 0);
    self->signal_type_u8    = self->intercom_parameters_aun[2].value_u8;
    self->vol_f32			= conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,0, 3);
    self->phase_inc = (2.0f * PI * self->rate_f32) / 48000.0;
    self->phase     = self->phase+self->phase_inc;
    float32_t mod = 0;
    float32_t lfo = 0;
    if ( self->signal_type_u8 == 0){
        // SIN
        mod = 0.5+sinf(self->phase);
    }else{
        // Triangle
        mod = generate_triangle_wave(self->phase);
        mod = 0.999*self->triag_prev_f32 + mod*0.001;
        self->triag_prev_f32 = mod;
    }

    // wrap around
    if (self->phase >= 2.0f * PI) {
        self->phase -= 2.0f * PI;
    }

    output_f32      = input_f32 * self->depth_f32 + input_f32*(1-self->depth_f32)*mod;
    output_f32 *=self->vol_f32;
    return output_f32;
};

