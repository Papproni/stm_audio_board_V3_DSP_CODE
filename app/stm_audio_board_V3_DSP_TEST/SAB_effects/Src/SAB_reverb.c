/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_reverb.h"

float32_t do_comb(float32_t inSample,float32_t* buffer,uint32_t* cf_ptr,float32_t cf_g, int cf_lim) {
	float32_t readback = buffer[*cf_ptr];
	float32_t new = readback*cf_g + inSample;
	buffer[*cf_ptr] = new;
	(*cf_ptr)++;
	if (*cf_ptr>=cf_lim) *cf_ptr = 0;
	return readback;
}


float32_t do_allpass(float32_t inSample,float32_t* buffer, uint32_t* ap_ptr, float32_t ap0_g, int ap_lim) {
	float32_t readback = buffer[*ap_ptr];
	readback += (-ap0_g) * inSample;
	float32_t new = readback*ap0_g + inSample;
	buffer[*ap_ptr] = new;
	(*ap_ptr)++;
	if (*ap_ptr >= ap_lim) *ap_ptr=0;
	return readback;
}

void SAB_reverb_delete( SAB_reverb_tst* self){
    // TODO
}


// Process Function for SAB_delay_tst
void SAB_reverb_init( SAB_reverb_tst* self){
    strcpy(self->intercom_fx_data.name, "Reverb");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 255;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
	add_parameter(&self->intercom_parameters_aun[0],"WET",	PARAM_TYPE_POT,	69);
	add_parameter(&self->intercom_parameters_aun[1],"TIME",	PARAM_TYPE_POT,	120);
	add_parameter(&self->intercom_parameters_aun[2],"MIX",	PARAM_TYPE_UNUSED,	85);
	add_parameter(&self->intercom_parameters_aun[3],"NONE",	PARAM_TYPE_UNUSED,	0);
	add_parameter(&self->intercom_parameters_aun[4],"NONE",	PARAM_TYPE_UNUSED,	10);
	add_parameter(&self->intercom_parameters_aun[5],"NONE",	PARAM_TYPE_UNUSED,	10);
    add_parameter(&self->intercom_parameters_aun[6],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE", PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

	//feedback defines as of Schroeder
	self->cf0_g = 	0.805;
	self->cf1_g	=	0.827; 
	self->cf2_g	=	0.783;
	self->cf3_g	=	0.764;
    self->ap0_g =	0.7;
	self->ap1_g =	0.7; 
	self->ap2_g =	0.7;
	//buffer-pointer
	self->cf0_p=0;
	self->cf1_p=0;
	self->cf2_p=0;
	self->cf3_p=0;
	self->ap0_p=0;
	self->ap1_p=0;
	self->ap2_p=0;

	// float32_t cfbuf0[l_CB0], cfbuf1[l_CB1], cfbuf2[l_CB2], cfbuf3[l_CB3];
    // float32_t apbuf0[l_AP0], apbuf1[l_AP1], apbuf2[l_AP2];
	self->cfbuf0 = sdram_malloc_float32_t_array(l_CB0);
	self->cfbuf1 = sdram_malloc_float32_t_array(l_CB1);
	self->cfbuf2 = sdram_malloc_float32_t_array(l_CB2);
	self->cfbuf3 = sdram_malloc_float32_t_array(l_CB3);
	self->apbuf0 = sdram_malloc_float32_t_array(l_AP0);
	self->apbuf1 = sdram_malloc_float32_t_array(l_AP1);
	self->apbuf2 = sdram_malloc_float32_t_array(l_AP2);
	// for(uint32_t i = 0; i<10000;i++){
	// 	if(i<l_CB0){
	// 		self->cfbuf0[i] = 0;
	// 	}
	// 	if(i<l_CB1){
	// 		self->cfbuf1[i] = 0;
	// 	}
	// 	if(i<l_CB2){
	// 		self->cfbuf2[i] = 0;
	// 	}
	// 	if(i<l_CB3){
	// 		self->cfbuf3[i] = 0;
	// 	}
	// 	if(i<l_AP0){
	// 		self->apbuf0[i] = 0;
	// 	}
	// 	if(i<l_AP1){
	// 		self->apbuf1[i] = 0;
	// 	}
	// 	if(i<l_AP2){
	// 		self->apbuf2[i] = 0;
	// 	}
	// }
};

// Process Function for SAB_delay_tst
float32_t SAB_reverb_process( SAB_reverb_tst* self, float input_f32){

	float32_t output_f32;

	self->wet_f32 	= conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0,1);
	self->time_f32 	= conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0,1);

	self->cf0_lim = (int)(self->time_f32*(l_CB0-1));
	self->cf1_lim = (int)(self->time_f32*(l_CB1-1));
	self->cf2_lim = (int)(self->time_f32*(l_CB2-1));
	self->cf3_lim = (int)(self->time_f32*(l_CB3-1));
	self->ap0_lim = (int)(self->time_f32*(l_AP0-1));
	self->ap1_lim = (int)(self->time_f32*(l_AP1-1));
	self->ap2_lim = (int)(self->time_f32*(l_AP2-1));

	
	//Do_Comb0(input_f32)
	float32_t comb0 = do_comb(input_f32,self->cfbuf0,&self->cf0_p,self->cf0_g,self->cf0_lim);
	//Do_Comb1(input_f32)
	float32_t comb1 = do_comb(input_f32,self->cfbuf1,&self->cf1_p,self->cf1_g,self->cf1_lim);
	//Do_Comb2(input_f32)
	float32_t comb2 = do_comb(input_f32,self->cfbuf2,&self->cf2_p,self->cf2_g,self->cf2_lim);
	//Do_Comb3(input_f32)
	float32_t comb3 = do_comb(input_f32,self->cfbuf3,&self->cf3_p,self->cf3_g,self->cf3_lim);
	float32_t newsample = (comb0+comb1+comb2+comb3)/4.0;
	//Do_Allpass0(newsample);
	float32_t allpass0 = do_allpass(newsample,self->apbuf0,&self->ap0_p,self->ap0_g,self->ap0_lim);
	//Do_Allpass1(newsample);
	float32_t allpass1 = do_allpass(allpass0,self->apbuf1,&self->ap1_p,self->ap1_g,self->ap1_lim);
	//Do_Allpass2(newsample);
	float32_t allpass2 = do_allpass(allpass1,self->apbuf2,&self->ap2_p,self->ap2_g,self->ap2_lim);
//
	output_f32 = (1.0-self->wet_f32)*input_f32 + self->wet_f32*allpass2;

    return output_f32;
};

