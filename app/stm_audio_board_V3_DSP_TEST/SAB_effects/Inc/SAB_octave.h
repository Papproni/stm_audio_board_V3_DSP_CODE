#ifndef GUITAR_EFFECT_OCTAVE_H
#define GUITAR_EFFECT_OCTAVE_H


/* //////////////////////////////////////////////////////////////
 * ****************  OCTAVE EFFECT CALCULATOR ******************
 * --------------------------------------------------------------
 * This library is created to calculate the octave ffect of a guitar signal.
 * The algorythm uses ERB-PS2 algorythm for OCTAVE UP
 * and uses the classic Resample algorythm for the OCTAVE DOWN
 *  -------------------------------------------------------------
 */

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "arm_math.h"
#include "sab_intercom.h"
#define numberofsubbands 55
// FILTER DEFINITIONS START





// FILTER DEFINITIONS END

typedef struct{
	float32_t sub_2_f32;
	float32_t sub_1_f32;
	float32_t clean_f32;
	float32_t up_1_f32;
	float32_t up_2_f32;
}octave_volume_tst;

typedef struct octave_effects_st{

	void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);
    void (*delete)(void*);
     
	fx_data_tst			intercom_fx_data;
	sab_fx_param_tun 	intercom_parameters_aun[12];

	// Volumes
	octave_volume_tst 	volumes_st;

	// I/O
	float32_t 			input_f32;
	float32_t 			output_f32;

	// output of dsp functions
	float32_t 			octave_up_1_f32;
	float32_t 			octave_up_2_f32;
	float32_t 			octave_down_1_f32;
	float32_t 			octave_down_2_f32;

	// create a filter to let only the AC signals to pass to the output
	arm_biquad_cascade_df2T_instance_f32 highpass_iir_50hz_octave_1_up_st;
	arm_biquad_cascade_df2T_instance_f32 highpass_iir_50hz_octave_1_down_st;
	float32_t subbandfilter_output[numberofsubbands];
	// saved states
	// x[n] , X[n-1], X[n-2]
	float32_t subbandfilter_dn[1];
	float32_t subbandfilter_dn1[1];
	float32_t subbandfilter_dn2[1];
	// Y[n-1] , Y[n-2]
	float32_t subbandfilter_yn1[numberofsubbands];
	float32_t subbandfilter_yn2[numberofsubbands];

	 float32_t subbandfilter_A1[numberofsubbands];
	 float32_t subbandfilter_A2[numberofsubbands];
	 float32_t subbandfilter_A[numberofsubbands];

	 float32_t subbandfilter_B0[numberofsubbands];
	 float32_t subbandfilter_B1[numberofsubbands];
	 float32_t subbandfilter_B2[numberofsubbands];
	 float32_t subbandfilter_B[numberofsubbands];
	
		// usefull filters
	float32_t subband_ones[numberofsubbands];
	float32_t subband_absolute_value[numberofsubbands];
	float32_t octave1_up_1;
	float32_t octave1_up_filtered;
	// octave Filter
	arm_biquad_cascade_df2T_instance_f32 highpass_iir_50hz;
	arm_biquad_cascade_df2T_instance_f32 highpass_iir_50hz_octave2;
	float32_t highpass_iir_50hz_coeffs[5];
	float32_t highpass_state[5];
	float32_t highpass_state_octave2[5];

	// SUB
	arm_biquad_cascade_df2T_instance_f32 biquad_filter_for_sub;
	float32_t biquad_filter_for_sub_coeffs_af32[5];
	float32_t biquad_filter_for_sub_states_af32[4];
	float32_t biquad_filter_for_sub_output_f32;


	float32_t Buf[10000];
	int BufSize;
	int Overlap;
	int WtrP;
	float Rd_P;
	float CrossFade;
	float Shift;

	// DSP functions
	 void (*subbandfilter_calculation) (struct octave_effects_st* self);
	// Calculate the octave 1 HIGHER
	void (*calc_octave_1_up) (struct octave_effects_st* self);
	// Calculate the octave 1 LOWER
	void (*calc_octave_1_down) (struct octave_effects_st* self);
	// Set volumes
	void (*set_volumes)(struct octave_effects_st* self);

	// Calculates the dsp functions on the input, returns the output
	int32_t (*callback) (struct octave_effects_st* self,int32_t input_i32);
}octave_effects_tst;

// creates the initialisation for the effect
void SAB_octave_init(octave_effects_tst* self);
//volatile octave_effects_tst octave_effect_st;

// Process Function for SAB_pitchshift_tst
float32_t SAB_octave_process( octave_effects_tst* self, float input_f32);

// Process Function for SAB_pitchshift_tst
void SAB_octave_delete( octave_effects_tst* self);

#endif
