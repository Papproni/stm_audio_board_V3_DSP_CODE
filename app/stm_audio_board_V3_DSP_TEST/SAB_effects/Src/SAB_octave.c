/*
 * guitar_effect_octave.c
 *
 *  Created on: Jan 25, 2024
 *      Author: pappr
 */

#include "SAB_octave.h"
// FILTER values are from audiotest4_ERB_PS2_naive2 matlab script
 // filter coefficients fs=48000khz

static const float32_t subbandfilter_a1[numberofsubbands]={
 		-1.99945743773269,
 		-1.99935942906841,
 		-1.99924062977670,
 		-1.99909708277235,
 		-1.99892410985062,
 		-1.99871618287911,
 		-1.99846677223317,
 		-1.99816816848975,
 		-1.99781127270592,
 		-1.99738534980214,
 		-1.99687773863020,
 		-1.99627351120883,
 		-1.99555507233332,
 		-1.99470168928216,
 		-1.99368893962368,
 		-1.99248806313769,
 		-1.99106520157565,
 		-1.98938050735340,
 		-1.98738709926732,
 		-1.98502983991592,
 		-1.98224390566903,
 		-1.97895311574385,
 		-1.97506798222950,
 		-1.97048343779300,
 		-1.96507619239353,
 		-1.95870166479969,
 		-1.95119042932896,
 		-1.94234411345253,
 		-1.93193067840176,
 		-1.91967901366415,
 		-1.90527277869813,
 		-1.88834343336828,
 		-1.86846241538006,
 		-1.84513245237305,
 		-1.81777804381746,
 		-1.78573522094381,
 		-1.74824080170997,
 		-1.70442151569457,
 		-1.65328359840612,
 		-1.59370376851867,
 		-1.52442293367053,
 		-1.44404455593239,
 		-1.35104038852965,
 		-1.24376731735276,
 		-1.12050035106663,
 		-0.979488440212929,
 		-0.819041780213102,
 		-0.637661520103793,
 		-0.434225203748800,
 		-0.208243466153559,
 		0.0397951757780011,
 		0.307975350044821,
 		0.592390391796649,
 		0.886463172281052,
 		1.18017689199105};
static const float32_t subbandfilter_a2[numberofsubbands]={
 		0.999623840823723,
 		0.999592083812410,
 		0.999557647639999,
 		0.999520306796363,
 		0.999479816929101,
 		0.999435913307174,
 		0.999388309169729,
 		0.999336693954506,
 		0.999280731400842,
 		0.999220057523294,
 		0.999154278453365,
 		0.999082968148916,
 		0.999005665973701,
 		0.998921874153418,
 		0.998831055119909,
 		0.998732628762124,
 		0.998625969611674,
 		0.998510404002805,
 		0.998385207262283,
 		0.998249601004902,
 		0.998102750636437,
 		0.997943763199395,
 		0.997771685739803,
 		0.997585504428058,
 		0.997384144736516,
 		0.997166473064829,
 		0.996931300315725,
 		0.996677388064530,
 		0.996403458142182,
 		0.996108206671829,
 		0.995790323872893,
 		0.995448521284569,
 		0.995081568475183,
 		0.994688341807231,
 		0.994267888432584,
 		0.993819509407934,
 		0.993342866651437,
 		0.992838119401008,
 		0.992306096857390,
 		0.991748514743871,
 		0.991168244482538,
 		0.990569644389679,
 		0.989958962429589,
 		0.989344819163852,
 		0.988738776872602,
 		0.988155995338887,
 		0.987615964937990,
 		0.987143291300058,
 		0.986768479981635,
 		0.986528630464629,
 		0.986467891703625,
 		0.986637451137907,
 		0.987094720833566,
 		0.987901246292733,
 		0.989118701024236};

static const float32_t subbandfilter_b0[numberofsubbands]={
 		0.000188079588138645,
 		0.000203958093794915,
 		0.000221176180000397,
 		0.000239846601818621,
 		0.000260091535449550,
 		0.000282043346413158,
 		0.000305845415135634,
 		0.000331653022746955,
 		0.000359634299578796,
 		0.000389971238353096,
 		0.000422860773317513,
 		0.000458515925542157,
 		0.000497167013149809,
 		0.000539062923291039,
 		0.000584472440045357,
 		0.000633685618937812,
 		0.000687015194163202,
 		0.000744797998597725,
 		0.000807396368858447,
 		0.000875199497549016,
 		0.000948624681781347,
 		0.00102811840030234,
 		0.00111415713009846,
 		0.00120724778597086,
 		0.00130792763174217,
 		0.00141676346758574,
 		0.00153434984213762,
 		0.00166130596773492,
 		0.00179827092890887,
 		0.00194589666408543,
 		0.00210483806355368,
 		0.00227573935771526,
 		0.00245921576240832,
 		0.00265582909638453,
 		0.00286605578370814,
 		0.00309024529603291,
 		0.00332856667428175,
 		0.00358094029949610,
 		0.00384695157130517,
 		0.00412574262806443,
 		0.00441587775873096,
 		0.00471517780516052,
 		0.00502051878520553,
 		0.00532759041807397,
 		0.00563061156369926,
 		0.00592200233055667,
 		0.00619201753100491,
 		0.00642835434997096,
 		0.00661576000918263,
 		0.00673568476768567,
 		0.00676605414818766,
 		0.00668127443104657,
 		0.00645263958321689,
 		0.00604937685363332,
 		0.00544064948788215
 };

static const float32_t subbandfilter_b1[numberofsubbands]={0.0};
static const float32_t subbandfilter_b2[numberofsubbands]={
 		-0.000188079588138645,
 		-0.000203958093794915,
 		-0.000221176180000397,
 		-0.000239846601818621,
 		-0.000260091535449550,
 		-0.000282043346413158,
 		-0.000305845415135634,
 		-0.000331653022746955,
 		-0.000359634299578796,
 		-0.000389971238353096,
 		-0.000422860773317513,
 		-0.000458515925542157,
 		-0.000497167013149809,
 		-0.000539062923291039,
 		-0.000584472440045357,
 		-0.000633685618937812,
 		-0.000687015194163202,
 		-0.000744797998597725,
 		-0.000807396368858447,
 		-0.000875199497549016,
 		-0.000948624681781347,
 		-0.00102811840030234,
 		-0.00111415713009846,
 		-0.00120724778597086,
 		-0.00130792763174217,
 		-0.00141676346758574,
 		-0.00153434984213762,
 		-0.00166130596773492,
 		-0.00179827092890887,
 		-0.00194589666408543,
 		-0.00210483806355368,
 		-0.00227573935771526,
 		-0.00245921576240832,
 		-0.00265582909638453,
 		-0.00286605578370814,
 		-0.00309024529603291,
 		-0.00332856667428175,
 		-0.00358094029949610,
 		-0.00384695157130517,
 		-0.00412574262806443,
 		-0.00441587775873096,
 		-0.00471517780516052,
 		-0.00502051878520553,
 		-0.00532759041807397,
 		-0.00563061156369926,
 		-0.00592200233055667,
 		-0.00619201753100491,
 		-0.00642835434997096,
 		-0.00661576000918263,
 		-0.00673568476768567,
 		-0.00676605414818766,
 		-0.00668127443104657,
 		-0.00645263958321689,
 		-0.00604937685363332,
 		-0.00544064948788215
 };



static void set_volumes(struct octave_effects_st* self){
	// reset output
	self->output_f32 = 0;

	// calculate the new output defined by the voluem params
	self->output_f32 += self->input_f32 		* self->volumes_st.clean_f32;
	self->output_f32 += self->octave_up_1_f32 	* self->volumes_st.up_1_f32;
	self->output_f32 += self->octave_down_1_f32 * self->volumes_st.sub_1_f32;
}


// SUBBAND FILTER FUNCTION - DIRECT FORM 2 - normalfunction exectime: ~6us
static inline void subbandfilter_calculation(struct octave_effects_st* self){
  float32_t input_f32=self->input_f32;
  // set d[n], d[n-1], d[n-2]
//   for(int i=0;i<numberofsubbands;i++){
// //			  subbandfilter_input[i]	= input_f32;
// 	  self->subbandfilter_dn2[i]		= self->subbandfilter_dn1[i];
// 	  self->subbandfilter_dn1[i]		= self->subbandfilter_dn[i];
// 	  self->subbandfilter_dn[i]		= self->subbandfilter_dn[i];

//   }
  self->subbandfilter_dn2[0] = self->subbandfilter_dn1[0];
  self->subbandfilter_dn1[0] = self->subbandfilter_dn[0];
  self->subbandfilter_dn[0] = input_f32;
  // A1 = a1*y[n-1]
  arm_mult_f32(subbandfilter_a1, self->subbandfilter_yn1, self->subbandfilter_A1, numberofsubbands);
  // A2 = a2*y[n-2]
  arm_mult_f32(subbandfilter_a2, self->subbandfilter_yn2, self->subbandfilter_A2, numberofsubbands);

  // A = A1+A2
  arm_add_f32(self->subbandfilter_A1, self->subbandfilter_A2, self->subbandfilter_A, numberofsubbands);

  // y_n=b0*d[n]+b1*d[n-1]+b2*d[n-2]

  // B1 = b1*x[n-1]
//   arm_mult_f32(subbandfilter_b1, self->subbandfilter_dn1, self->subbandfilter_B1, numberofsubbands);
  arm_scale_f32(subbandfilter_b1,self->subbandfilter_dn1[0],self->subbandfilter_B1,numberofsubbands);
  // B2 = b2*x[n-2]
//   arm_mult_f32(subbandfilter_b2, self->subbandfilter_dn2, self->subbandfilter_B2, numberofsubbands);
  arm_scale_f32(subbandfilter_b2,self->subbandfilter_dn2[0],self->subbandfilter_B2,numberofsubbands);
  // B1+B2
  arm_add_f32(self->subbandfilter_B1, self->subbandfilter_B2, self->subbandfilter_B, numberofsubbands);

  // B0 = b0*x[n]
//   arm_mult_f32(subbandfilter_b0, self->subbandfilter_dn, self->subbandfilter_B0, numberofsubbands);
  arm_scale_f32(subbandfilter_b0,input_f32,self->subbandfilter_B0,numberofsubbands);

  // y=B0+B1+B2
  arm_add_f32(self->subbandfilter_B, self->subbandfilter_B0, self->subbandfilter_B, numberofsubbands);

  // y[n]= B - A
  arm_sub_f32(self->subbandfilter_B,self->subbandfilter_A, self->subbandfilter_output, numberofsubbands);
//   for(int i=0;i<numberofsubbands;i++){
// 	  self->subbandfilter_yn2[i]		= self->subbandfilter_yn1[i];
// 	  self->subbandfilter_yn1[i]		= self->subbandfilter_output[i];
//    }
   arm_copy_f32(self->subbandfilter_yn1,self->subbandfilter_yn2,numberofsubbands);
   arm_copy_f32(self->subbandfilter_output,self->subbandfilter_yn1,numberofsubbands);

}

// Calculate the octave 1 HIGHER
static void algorithm_octave_1_up(struct octave_effects_st* self){
	// TODO
}

// Calculate the octave 1 LOWER
static void algorithm_octave_1_down(struct octave_effects_st* self){
	// TODO
}


static inline void octave1up(struct octave_effects_st* self){
	// get absolute values of subbands
	arm_abs_f32(self->subbandfilter_output, self->subband_absolute_value, numberofsubbands);


	// add the octave subbands together
	arm_dot_prod_f32(self->subband_absolute_value, self->subband_ones, numberofsubbands, &self->octave1_up_1);

	// filter the DC component out
	arm_biquad_cascade_df2T_f32(&self->highpass_iir_50hz, &self->octave1_up_1, &self->octave1_up_filtered, 1);
}

static int32_t callback_octave_effect(struct octave_effects_st* self,int32_t input_i32){
	// 1. calculate octaves

	// 2. set volumes

	// 3. return value


	// LEGACY CODE
	self->input_f32 = (float32_t)input_i32;
	// +1 octave
	subbandfilter_calculation(self);
	octave1up(self);
	// save result
	self->octave_up_1_f32 = self->octave1_up_filtered;
	
	//		 save result
	self->octave_up_2_f32 = self->octave1_up_filtered;

	// Write to DAC
	self->output_f32 =	(int32_t)self->octave_up_1_f32*self->volumes_st.up_1_f32 +
						(int32_t)self->octave_up_2_f32*self->volumes_st.up_2_f32 +
						(int32_t)(self->input_f32*self->volumes_st.clean_f32);

	return self->output_f32;
}

static float32_t Do_PitchShift(struct octave_effects_st* self, float32_t sample_f32) {

	int RdPtr_Int ;
	int RdPtr_Int2;
	float Rd0;
	float Rd1;
	int rel;
	float32_t sum;

	//write to ringbuffer
	self->Buf[self->WtrP] = sample_f32;

	//read fractional readpointer and generate 0° and 180° read-pointer in integer
	 RdPtr_Int = roundf(self->Rd_P);
	 RdPtr_Int2 = 0;
	if (RdPtr_Int >= self->BufSize/2) RdPtr_Int2 = RdPtr_Int - (self->BufSize/2);
	else RdPtr_Int2 = RdPtr_Int + (self->BufSize/2);

	//read the two samples...
	Rd0 = (float) self->Buf[RdPtr_Int];
	Rd1 = (float) self->Buf[RdPtr_Int2];

	//Check if first readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to second read-pointer
	if (self->Overlap >= (self->WtrP-RdPtr_Int) && (self->WtrP-RdPtr_Int) >= 0 && self->Shift!=1.0f) {
		 rel = self->WtrP-RdPtr_Int;
		self->CrossFade = ((float)rel)/(float)self->Overlap;
	}
	else if (self->WtrP-RdPtr_Int == 0) self->CrossFade = 0.0f;

	//Check if second readpointer starts overlap with write pointer?
	// if yes -> do cross-fade to first read-pointer
	if (self->Overlap >= (self->WtrP-RdPtr_Int2) && (self->WtrP-RdPtr_Int2) >= 0 && self->Shift!=1.0f) {
			 rel = self->WtrP-RdPtr_Int2;
			self->CrossFade = 1.0f - ((float)rel)/(float)self->Overlap;
		}
	else if (self->WtrP-RdPtr_Int2 == 0) self->CrossFade = 1.0f;


	//do cross-fading and sum up
	sum = (Rd0*self->CrossFade + Rd1*(1.0f-self->CrossFade));

	//increment fractional read-pointer and write-pointer
	self->Rd_P += self->Shift;
	self->WtrP++;
	if (self->WtrP == self->BufSize) self->WtrP = 0;
	if (roundf(self->Rd_P) >= self->BufSize) self->Rd_P = 0.0f;

	return sum;
}

// Process Function for SAB_pitchshift_tst
float32_t SAB_octave_process( octave_effects_tst* self, float input_f32){
		self->volumes_st.sub_1_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 10);
	self->volumes_st.clean_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0, 10);
	self->volumes_st.up_1_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 20);
	self->volumes_st.up_2_f32 = self->volumes_st.up_1_f32;
	// LEGACY CODE
	self->input_f32 = (float32_t)input_f32;
	// +1 octave
	subbandfilter_calculation(self);
	octave1up(self);
	// save result
	self->octave_up_1_f32 = self->octave1_up_filtered;

	float32_t filtered_input_f32;
	arm_biquad_cascade_df2T_f32(&self->biquad_filter_for_sub, &input_f32, &filtered_input_f32, 1);
	self->octave_down_1_f32 = Do_PitchShift(self,filtered_input_f32);
	

// Write to DAC
	self->output_f32 =	self->octave_down_1_f32*self->volumes_st.sub_1_f32+
						self->octave_up_1_f32*self->volumes_st.up_1_f32 +
						(input_f32*self->volumes_st.clean_f32);
	return self->output_f32;
}

// Process Function for SAB_pitchshift_tst
void SAB_octave_delete( octave_effects_tst* self){

}


void SAB_octave_init(octave_effects_tst* self){
	// SETUP DATA FOR GUI START------------------------------
	strcpy(self->intercom_fx_data.name, "Octave");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_ON;

	// PARAMS:
	add_parameter(&self->intercom_parameters_aun[0],"SUB",PARAM_TYPE_POT,69);
	add_parameter(&self->intercom_parameters_aun[1],"VOL",PARAM_TYPE_POT,120);
	add_parameter(&self->intercom_parameters_aun[2],"UP",PARAM_TYPE_POT,85);
    add_parameter(&self->intercom_parameters_aun[3],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);
	
	// SETUP DATA FOR GUI END ------------------------------
	float32_t local_50hz_coeffs[] = {0.99538200, -1.99076399, 0.99538200, 1.99074267, -0.99078531};
	memcpy(self->highpass_iir_50hz_coeffs,local_50hz_coeffs,sizeof(float32_t)*5);
	arm_biquad_cascade_df2T_init_f32(&self->highpass_iir_50hz, 1, &self->highpass_iir_50hz_coeffs, &self->highpass_state);
	arm_biquad_cascade_df2T_init_f32(&self->highpass_iir_50hz_octave2, 1, &self->highpass_iir_50hz_coeffs, &self->highpass_state_octave2);
	// assign function pointers
	self->set_volumes			= set_volumes;
	self->calc_octave_1_up 		= algorithm_octave_1_down;
	self->calc_octave_1_down 	= algorithm_octave_1_up;
	self->callback 				= callback_octave_effect;

	self->volumes_st.up_1_f32 = 1;
	self->volumes_st.up_2_f32 = 1;
	self->volumes_st.clean_f32      = 1;
	for(int i=0; i<numberofsubbands;i++){
		self->subband_ones[i] = 1;
		self->subbandfilter_yn1[i] 		= 	0;
		self->subbandfilter_yn2[i] 		= 	0;
	}

	self->subbandfilter_dn2[0]		=	0;
	self->subbandfilter_dn1[0]		=	0;
	self->subbandfilter_dn[0]		=	0;
	// 1kkhz LPF
	float Q = 3;
	float Freq = 3000;
	float omega = 2.0f * 3.14 * Freq / 48000;
    float alpha = sinf(omega) / (2.0f * Q);
    float cos_omega = cosf(omega);
	self->biquad_filter_for_sub_coeffs_af32[0]= (1.0f - cos_omega) / 2.0f 	/ (1.0 + alpha);
	self->biquad_filter_for_sub_coeffs_af32[1]= 1.0f - cos_omega	/ (1.0 + alpha);
	self->biquad_filter_for_sub_coeffs_af32[2]= (1.0f - cos_omega) / 2.0f 	/ (1.0 + alpha);
	self->biquad_filter_for_sub_coeffs_af32[3]= -2.0f * cos_omega	/ (1.0 + alpha);
	self->biquad_filter_for_sub_coeffs_af32[4]= (1.0f - alpha)	/ (1.0 + alpha);
	self->biquad_filter_for_sub_coeffs_af32[3]=-self->biquad_filter_for_sub_coeffs_af32[3];
	self->biquad_filter_for_sub_coeffs_af32[4]=-self->biquad_filter_for_sub_coeffs_af32[4];
	
	// SUB
	arm_biquad_cascade_df2T_init_f32(&self->biquad_filter_for_sub, 1, &self->biquad_filter_for_sub_coeffs_af32, &self->biquad_filter_for_sub_states_af32);

	for(int i=0 ;i<5;i++){
		self->biquad_filter_for_sub_states_af32[i] = 0;
	}
	self->Shift = 0.5;
	self->BufSize = 5000;
	self->Overlap = 2500;
	self->Rd_P = 0;
	self->WtrP = 0;
	self->CrossFade = 0;
	// zero out the IIR filter states
}
