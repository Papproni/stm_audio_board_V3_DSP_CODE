/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_pitchdetector.h"

// Convert uint8_t sign[] (0/1) -> q7_t +/-1
static void convert_sign_to_q7(const uint8_t *sign, q7_t *out, int N)
{
    for (int i = 0; i < N; i++)
        out[i] = sign[i] ? 1 : -1;   // Q7 in range [-128..127] but value is tiny so safe
}

static float32_t pitchdet_bitwise_autocorr_cmsis(
        const q7_t *sign_q7,
        int N,
        float sample_rate)
{
    const float MIN_FREQ = 40.0f;
    const float MAX_FREQ = 500.0f;

    int minLag = (int)(sample_rate / MAX_FREQ);
    int maxLag = (int)(sample_rate / MIN_FREQ);

    if (minLag < 1) minLag = 1;
    if (maxLag > N - 3) maxLag = N - 3;

    int window = N - maxLag - 1;
    if (window < 10) return 0.0f;

    float bestScore = -1e9f;
    int   bestLag = 0;

    int32_t tmpScore;

    // --------------------------------------------------------------------
    // 1. Find best lag using vectorized dot product
    // --------------------------------------------------------------------
    for (int lag = minLag; lag <= maxLag; lag++)
    {
        tmpScore = 0;

        // CMSIS: fast dot product (SIMD) of q7 vectors
        arm_dot_prod_q7(sign_q7, &sign_q7[lag], window, &tmpScore);

        // normalize to [-1..1]
        float normScore = (float)tmpScore / (float)(window);

        if (normScore > bestScore)
        {
            bestScore = normScore;
            bestLag = lag;
        }
    }

    if (bestLag <= 1) return 0.0f;

    // --------------------------------------------------------------------
    // 2. Sub-sample parabolic interpolation (3-dot-products)
    // --------------------------------------------------------------------
    float Rm1, R0, R1;

    // lag - 1
    arm_dot_prod_q7(sign_q7, &sign_q7[bestLag - 1], window, &tmpScore);
    Rm1 = (float)tmpScore / (float)window;

    // lag
    arm_dot_prod_q7(sign_q7, &sign_q7[bestLag], window, &tmpScore);
    R0 = (float)tmpScore / (float)window;

    // lag + 1
    arm_dot_prod_q7(sign_q7, &sign_q7[bestLag + 1], window, &tmpScore);
    R1 = (float)tmpScore / (float)window;

    // --------------------------------------------------------------------
    // 3. Parabolic interpolation
    // --------------------------------------------------------------------
    float denom = (Rm1 - 2.0f * R0 + R1);
    float delta = 0.0f;

    if (fabsf(denom) > 1e-6f)
        delta = 0.5f * (Rm1 - R1) / denom;

    float refinedLag = bestLag + delta;

    if (refinedLag < minLag) refinedLag = minLag;
    if (refinedLag > maxLag) refinedLag = maxLag;

    if (bestScore < 0.10f) return 0.0f;

    return sample_rate / refinedLag;
}

static void compute_sign_bits_cmsis(const float32_t *in,
                                    uint8_t *sign_out,
                                    float32_t average,
                                    uint32_t length)
{
    static float32_t temp[ PITCH_DETECTOR_BUFFER_SIZE ];

    // ---------------------------------------------------------
    // 1. Center signal: temp[i] = in[i] - average
    // ---------------------------------------------------------
    float32_t neg_avg = -average;
    arm_offset_f32(in, neg_avg, temp, length);


    // ---------------------------------------------------------
    // 2. Vectorized sign extraction
    //
    // Trick:
    // sign = (x >= 0) → 1
    // sign = (x < 0)  → 0
    //
    // Equivalent to:
    // sign = ( (x + |x|) > 0 )
    //
    // because:
    //   x >= 0  → x+|x| = 2x > 0
    //   x < 0   → x+|x| = 0
    // ---------------------------------------------------------
    float32_t absbuf, sum;
    for (uint32_t i = 0; i < length; i++)
    {
        absbuf = fabsf(temp[i]);                // CMSIS uses hardware FABS
        sum    = temp[i] + absbuf;              // ≥0 → positive, <0 → 0
        sign_out[i] = (sum > 0.0f) ? 1U : 0U;    // no branch mispredict (constant condition)
    }
}

static float32_t pitchdet_bitwise_autocorr(const uint8_t *sign, int N, float sample_rate)
{
    const float MIN_FREQ = 50.0f;
    const float MAX_FREQ = 5000.0f;

    int minLag = (int)(sample_rate / MAX_FREQ);
    int maxLag = (int)(sample_rate / MIN_FREQ);

    if (minLag < 1) minLag = 1;
    if (maxLag > N - 3) maxLag = N - 3;   // allow lag-1 and lag+1

    int window = N - maxLag - 1;
    if (window < 10) return 0.0f;

    float bestScore = -1e9f;
    int bestLag = 0;

    int lag;
    int i;

    // --------------------------------------------------------------------
    // 1. Find best lag using normalized correlation
    // --------------------------------------------------------------------
    for (lag = minLag; lag <= maxLag; lag++)
    {
        int score = 0;

        for (i = 0; i < window; i++)
        {
            score += (sign[i] == sign[i + lag]) ? 1 : -1;
        }

        float normScore = (float)score / (float)window;

        if (normScore > bestScore)
        {
            bestScore = normScore;
            bestLag = lag;
        }
    }

    if (bestLag <= 1) return 0.0f;

    // --------------------------------------------------------------------
    // 2. Parabolic interpolation for sub-sample accuracy
    //    Compute correlation at lag-1, lag, lag+1
    // --------------------------------------------------------------------
    float Rm1 = 0.0f;
    float R0  = 0.0f;
    float R1  = 0.0f;

    {
        int score = 0;
        for (i = 0; i < window; i++)
            score += (sign[i] == sign[i + bestLag - 1]) ? 1 : -1;
        Rm1 = (float)score / (float)window;
    }

    {
        int score = 0;
        for (i = 0; i < window; i++)
            score += (sign[i] == sign[i + bestLag]) ? 1 : -1;
        R0 = (float)score / (float)window;
    }

    {
        int score = 0;
        for (i = 0; i < window; i++)
            score += (sign[i] == sign[i + bestLag + 1]) ? 1 : -1;
        R1 = (float)score / (float)window;
    }

    // --------------------------------------------------------------------
    // 3. Parabolic peak estimation
    // --------------------------------------------------------------------
    float denom = (Rm1 - 2.0f * R0 + R1);
    float delta = 0.0f;

    if (fabsf(denom) > 1e-6f)
        delta = 0.5f * (Rm1 - R1) / denom;

    float refinedLag = (float)bestLag + delta;

    if (refinedLag < (float)minLag) refinedLag = (float)minLag;
    if (refinedLag > (float)maxLag) refinedLag = (float)maxLag;

    // --------------------------------------------------------------------
    // 4. Reject bad signals (prevents false detections)
    // --------------------------------------------------------------------
    if (bestScore < 0.10f)   // at least 10% sign agreement
        return 0.0f;

    // --------------------------------------------------------------------
    // 5. Convert lag → frequency
    // --------------------------------------------------------------------
    return sample_rate / refinedLag;
}



void SAB_pitchdetector_delete( SAB_pitchdetector_tst* self){
    // TODO
}

arm_biquad_casd_df1_inst_f32 lp500;
float32_t lpState[4];  // 4 floats for 1 biquad stage

// Coeffs for 500 Hz LPF @ 48 kHz
static const float32_t lpCoeffs[5] =
{
    -0.0010257f,   // b0
    -0.0020514f,   // b1
    -0.0010257f,   // b2
   1.9021130f,   // a1
    -0.9070295f    // a2
};



// Process Function for SAB_pitchdetector_tst
void SAB_pitchdetector_init( SAB_pitchdetector_tst* self){
    strcpy(self->intercom_fx_data.name, "Ptchdtcr");
	self->intercom_fx_data.color[0] = 255; 	// R
	self->intercom_fx_data.color[1] = 0;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    
    add_parameter(&self->intercom_parameters_aun[0],"A4",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[2],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[3],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[4],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[5],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[6],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

	self->input_raw_cntr_u32 = 0;

	arm_biquad_cascade_df1_init_f32(&lp500, 1, lpCoeffs, lpState);
};


float32_t freq_f32;


// Process Function for SAB_pitchdetector_tst
float32_t SAB_pitchdetector_process( SAB_pitchdetector_tst* self, float input_f32){
	// Step1: Get data and store it in buffer
	float32_t filtered;
	arm_biquad_cascade_df1_f32(&lp500, &input_f32, &filtered, 1);
	self->input_raw_af32[self->input_raw_cntr_u32] = filtered;
	self->input_raw_cntr_u32++;
	if(self->input_raw_cntr_u32 >= PITCH_DETECTOR_BUFFER_SIZE){
		self->input_raw_cntr_u32 = 0;
		// Calculate average
		arm_rms_f32(self->input_raw_af32,PITCH_DETECTOR_BUFFER_SIZE,&self->average_f32);
		// -----------------------------------------------------
        // 3) Remove mean -> sign bits
        // -----------------------------------------------------
		
		// arm_offset_f32(self->input_raw_af32, -self->average_f32, self->centered_f32, PITCH_DETECTOR_BUFFER_SIZE);
        // for (int i = 0; i < PITCH_DETECTOR_BUFFER_SIZE; i++) {
        //     self->sign_bits[i] = (self->centered_f32[i] >= 0.0f) ? 1U : 0U;
        // }

		for (int i = 0; i < PITCH_DETECTOR_BUFFER_SIZE; i++) {
            float32_t centered = self->input_raw_af32[i] - self->average_f32;
            self->sign_bits[i] = (centered >= 0.0f) ? 1U : 0U;
        }

        // -----------------------------------------------------
        // 4) Autocorrelation pitch detection
        // -----------------------------------------------------
        self->detected_freq_f32 = pitchdet_bitwise_autocorr_cmsis(
            self->sign_bits,
            PITCH_DETECTOR_BUFFER_SIZE,
            48000.0f                      // sample rate
        );

        if(self->detected_freq_f32>1){
			freq_f32 = self->detected_freq_f32;
		}
        

		
		// Step2: If buffer is full, calculate the mean values

		// Step3: Determine Zero corrsings

		// Step4: Use the autocorrelate function to get frequency
	}






    // self->vol_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 3);
    // self->mix_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,1, 0);
    // self->shift_f32 = (int)conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,-12, 13);
	// self->Shift =  powf(2.0f, (float)self->shift_f32 / 12.0f);
    // float32_t output_f32;
    
    // output_f32 = self->vol_f32 * ((1- self->mix_f32) *Do_pitchdetector(self,input_f32) + self->mix_f32*input_f32);
	float32_t output_f32 = 0;
    return output_f32;
};

