/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_pitchshift.h"

void SAB_pitchshift_delete( SAB_pitchshift_tst* self){
    // TODO
}

static float32_t Do_PitchShift(SAB_pitchshift_tst* self, float32_t sample_f32) {

	//write to ringbuffer
	self->Buf[self->WtrP] = sample_f32;

	//read fractional readpointer and generate 0° and 180° read-pointer in integer
	int RdPtr_Int = roundf(self->Rd_P);
	int RdPtr_Int2 = 0;
	if (RdPtr_Int >= self->BufSize/2) RdPtr_Int2 = RdPtr_Int - (self->BufSize/2);
	else RdPtr_Int2 = RdPtr_Int + (self->BufSize/2);

	//read the two samples...
	float Rd0 = (float) self->Buf[RdPtr_Int];
	float Rd1 = (float) self->Buf[RdPtr_Int2];

	//Check if first readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to second read-pointer
	if (self->Overlap >= (self->WtrP-RdPtr_Int) && (self->WtrP-RdPtr_Int) >= 0 && self->Shift!=1.0f) {
		int rel = self->WtrP-RdPtr_Int;
		self->CrossFade = ((float)rel)/(float)self->Overlap;
	}
	else if (self->WtrP-RdPtr_Int == 0) self->CrossFade = 0.0f;

	//Check if second readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to first read-pointer
	if (self->Overlap >= (self->WtrP-RdPtr_Int2) && (self->WtrP-RdPtr_Int2) >= 0 && self->Shift!=1.0f) {
			int rel = self->WtrP-RdPtr_Int2;
			self->CrossFade = 1.0f - ((float)rel)/(float)self->Overlap;
		}
	else if (self->WtrP-RdPtr_Int2 == 0) self->CrossFade = 1.0f;


	//do cross-fading and sum up
	float32_t sum = (Rd0*self->CrossFade + Rd1*(1.0f-self->CrossFade));

	//increment fractional read-pointer and write-pointer
	self->Rd_P += self->Shift;
	self->WtrP++;
	if (self->WtrP == self->BufSize) self->WtrP = 0;
	if (roundf(self->Rd_P) >= self->BufSize) self->Rd_P = 0.0f;

	return sum;
}


// Process Function for SAB_pitchshift_tst
void SAB_pitchshift_init( SAB_pitchshift_tst* self){
    strcpy(self->intercom_fx_data.name, "Pitcshft");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"VOL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"MIX",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[2],"SFT",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"GAIN",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);


    self->Shift = 0.5;
	self->BufSize = 4000;
	self->Overlap = 1000;
	self->Rd_P = 0;
	self->WtrP = 0;
	self->CrossFade = 0;
};

// Process Function for SAB_pitchshift_tst
float32_t SAB_pitchshift_process( SAB_pitchshift_tst* self, float input_f32){

    self->vol_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 3);
    self->mix_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,1, 0);
    self->shift_f32 = (int)conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,-12, 12);
     self->Shift =  powf(2.0f, (float)self->shift_f32 / 12.0f);
    float32_t output_f32;
    
    output_f32 = self->vol_f32 * ((1- self->mix_f32) *Do_PitchShift(self,input_f32) + self->mix_f32*input_f32);

    return output_f32;
};

