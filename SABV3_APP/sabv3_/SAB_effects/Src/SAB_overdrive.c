/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_overdrive.h"

void SAB_overdrive_delete( SAB_overdrive_tst* self){
    // TODO
}

typedef enum {
    LPF,
    HPF,
    BANDPASS,
    NOTCH
} FilterType;
typedef struct {
    float b0, b1, b2; // Numerator coefficients
    float a1, a2;     // Denominator coefficients
} BiquadCoeffs;

static void calculate_biquad_coeffs(float32_t* coeffs_af32,FilterType type, float freq, float Q, float sample_rate) {
    float omega = 2.0f * 3.14159265358979323846 * freq / sample_rate;
    float alpha = sinf(omega) / (2.0f * Q);
    float cos_omega = cosf(omega);

    BiquadCoeffs coeffs;

    switch (type) {
        case LPF:
            coeffs.b0 = (1.0f - cos_omega) / 2.0f;
            coeffs.b1 = 1.0f - cos_omega;
            coeffs.b2 = (1.0f - cos_omega) / 2.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case HPF:
            coeffs.b0 = (1.0f + cos_omega) / 2.0f;
            coeffs.b1 = -(1.0f + cos_omega);
            coeffs.b2 = (1.0f + cos_omega) / 2.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case BANDPASS:
            coeffs.b0 = alpha;
            coeffs.b1 = 0.0f;
            coeffs.b2 = -alpha;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;

        case NOTCH:
            coeffs.b0 = 1.0f;
            coeffs.b1 = -2.0f * cos_omega;
            coeffs.b2 = 1.0f;
            coeffs.a1 = -2.0f * cos_omega;
            coeffs.a2 = 1.0f - alpha;
            break;
    }

    // Normalize coefficients
    float a0 = 1.0f + alpha;
    coeffs.b0 /= a0;
    coeffs.b1 /= a0;
    coeffs.b2 /= a0;
    coeffs.a1 /= a0;
    coeffs.a2 /= a0;

    coeffs_af32[0]=coeffs.b0;
    coeffs_af32[1]=coeffs.b1;
    coeffs_af32[2]=coeffs.b2;
    coeffs_af32[3]=-coeffs.a1;
    coeffs_af32[4]=-coeffs.a2;
}

// GENERATED

static float sab_biquad_process(sab_biquad_tst *f, float x)
{
    float y = f->b0 * x + f->z1;
    f->z1 = f->b1 * x - f->a1 * y + f->z2;
    f->z2 = f->b2 * x - f->a2 * y;
    return y;
}

static void sab_biquad_set_lpf(sab_biquad_tst *f, float fs, float fc, float q)
{
    float w0 = 2.0f * SAB_PI * fc / fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0f * q);

    float b0 = (1.0f - c) * 0.5f;
    float b1 = 1.0f - c;
    float b2 = (1.0f - c) * 0.5f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * c;
    float a2 = 1.0f - alpha;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
}

static void sab_biquad_set_hpf(sab_biquad_tst *f, float fs, float fc, float q)
{
    float w0 = 2.0f * SAB_PI * fc / fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0f * q);

    float b0 = (1.0f + c) * 0.5f;
    float b1 = -(1.0f + c);
    float b2 = (1.0f + c) * 0.5f;
    float a0 = 1.0f + alpha;
    float a1 = -2.0f * c;
    float a2 = 1.0f - alpha;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
}

static void sab_biquad_set_peak(sab_biquad_tst *f, float fs, float fc, float q, float gain_db)
{
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * SAB_PI * fc / fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0f * q);

    float b0 = 1.0f + alpha * A;
    float b1 = -2.0f * c;
    float b2 = 1.0f - alpha * A;
    float a0 = 1.0f + alpha / A;
    float a1 = -2.0f * c;
    float a2 = 1.0f - alpha / A;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
}

static void sab_biquad_set_highshelf(sab_biquad_tst *f, float fs, float fc, float gain_db)
{
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * SAB_PI * fc / fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0f * 0.7071f);
    float beta = 2.0f * sqrtf(A) * alpha;

    float b0 = A * ((A + 1.0f) + (A - 1.0f) * c + beta);
    float b1 = -2.0f * A * ((A - 1.0f) + (A + 1.0f) * c);
    float b2 = A * ((A + 1.0f) + (A - 1.0f) * c - beta);
    float a0 = (A + 1.0f) - (A - 1.0f) * c + beta;
    float a1 = 2.0f * ((A - 1.0f) - (A + 1.0f) * c);
    float a2 = (A + 1.0f) - (A - 1.0f) * c - beta;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
}

static void sab_biquad_set_lowshelf(sab_biquad_tst *f, float fs, float fc, float gain_db)
{
    float A = powf(10.0f, gain_db / 40.0f);
    float w0 = 2.0f * SAB_PI * fc / fs;
    float c = cosf(w0);
    float s = sinf(w0);
    float alpha = s / (2.0f * 0.7071f);
    float beta = 2.0f * sqrtf(A) * alpha;

    float b0 = A * ((A + 1.0f) - (A - 1.0f) * c + beta);
    float b1 = 2.0f * A * ((A - 1.0f) - (A + 1.0f) * c);
    float b2 = A * ((A + 1.0f) - (A - 1.0f) * c - beta);
    float a0 = (A + 1.0f) + (A - 1.0f) * c + beta;
    float a1 = -2.0f * ((A - 1.0f) + (A + 1.0f) * c);
    float a2 = (A + 1.0f) + (A - 1.0f) * c - beta;

    f->b0 = b0 / a0;
    f->b1 = b1 / a0;
    f->b2 = b2 / a0;
    f->a1 = a1 / a0;
    f->a2 = a2 / a0;
}

static void sab_biquad_reset(sab_biquad_tst *f)
{
    f->z1 = 0.0f;
    f->z2 = 0.0f;
}

static float sab_red_led_clip(float x)
{
    const float vf_pos = 0.62f;
    const float vf_neg = 0.55f;
    const float knee   = 0.16f;

    if (x > vf_pos)
        return vf_pos + knee * tanhf((x - vf_pos) / knee);

    if (x < -vf_neg)
        return -vf_neg + knee * tanhf((x + vf_neg) / knee);

    return x;
}

static float sab_dual_red_led_clip(float x)
{
    const float vf_pos = 0.62f*2;
    const float vf_neg = 0.55f*2;
    const float knee   = 0.16f*2;

    if (x > vf_pos)
        return vf_pos + knee * tanhf((x - vf_pos) / knee);

    if (x < -vf_neg)
        return -vf_neg + knee * tanhf((x + vf_neg) / knee);

    return x;
}

static float sab_tl072_stage(float x, float gain)
{
    x *= gain;

    // TL072 rails are not infinite. This avoids digital hard clipping.
    x = 2.2f * tanhf(x / 2.2f);

    return x;
}

static float sab_sag_process(SAB_overdrive_tst *fx, float x)
{
    float absx = fabsf(x);

    // envelope follower
    fx->env = fx->env * 0.9992f + absx * 0.0008f;

    // gain drops when signal is large
    float sag = 1.0f / (1.0f + 0.65f * fx->env);

    return x * sag;
}

void sab_beod_update(SAB_overdrive_tst *fx)
{
    float fs = fx->fs;

    float tight_fc = 152.0f * powf(3388.0f / 152.0f, fx->tight);

    float gain_db     = fx->gain * 34.0f;
    float bass_db     = (fx->bass_knob     - 0.5f) * 14.0f;
    float mid_db      = (fx->mid_knob      - 0.5f) * 16.0f;
    float treble_db   = (fx->treble_knob   - 0.5f) * 16.0f;
    float presence_db = (fx->presence_knob - 0.5f) * 14.0f;

    sab_biquad_set_hpf(&fx->tight_hpf, fs, tight_fc, 0.707f);
    sab_biquad_set_lpf(&fx->pre_lpf, fs, 16000.0f, 0.707f);

    sab_biquad_set_lpf(&fx->stage1_lpf, fs, 7500.0f, 0.707f);
    sab_biquad_set_hpf(&fx->inter_hpf, fs, 140.0f, 0.707f);

    // BE-OD-ish growl: reduce low-mid mud, push upper mids
    sab_biquad_set_peak(&fx->inter_mid, fs, 850.0f, 0.8f, -3.0f + fx->gain * 2.0f);

    sab_biquad_set_lpf(&fx->stage2_lpf, fs, 10000.0f, 0.707f);

    sab_biquad_set_lowshelf(&fx->bass, fs, 120.0f, bass_db);
    sab_biquad_set_peak(&fx->mid, fs, 750.0f, 0.8f, mid_db);
    sab_biquad_set_highshelf(&fx->treble, fs, 2500.0f, treble_db);
    sab_biquad_set_highshelf(&fx->presence, fs, 4200.0f, presence_db);

    // (void)gain_db;
}




// Process Function for SAB_overdrive_tst
void SAB_overdrive_init( SAB_overdrive_tst* self){
    strcpy(self->intercom_fx_data.name, "Overdrive");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 255;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:

    add_parameter(&self->intercom_parameters_aun[0],"GAIN",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[1],"TGHT",PARAM_TYPE_POT,69 );
    add_parameter(&self->intercom_parameters_aun[2],"BASS",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[3],"MID",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[4],"TRE",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[5],"PRS",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[6],"VOL",PARAM_TYPE_POT,69);
    add_parameter(&self->intercom_parameters_aun[7],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[8],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[9],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[10],"NONE",PARAM_TYPE_UNUSED,69);
    add_parameter(&self->intercom_parameters_aun[11],"NONE",PARAM_TYPE_UNUSED,69);

    arm_biquad_cascade_df2T_init_f32(&self->biquad_filter, 1, (float32_t*)&self->biquad_filter_coeffs_af32, (float32_t*)&self->biquad_filter_states_af32);
    self->biquad_filter_output_f32  = 0;


    self->ths_f32 = INT32_MAX/5/4;

    self->fs = 48000;
    self->env = 0.0f;
    self->gain = 0.75f;
    self->tight = 0.6f;
    self->bass_knob = 0.5f;
    self->mid_knob = 0.5f;
    self->treble_knob = 0.5f;
    self->presence_knob = 0.5f;
    self->volume = 0.5f;

    sab_biquad_reset(&self->tight_hpf);
    sab_biquad_reset(&self->pre_lpf);
    sab_biquad_reset(&self->stage1_lpf);
    sab_biquad_reset(&self->inter_hpf);
    sab_biquad_reset(&self->inter_mid);
    sab_biquad_reset(&self->stage2_lpf);
    sab_biquad_reset(&self->bass);
    sab_biquad_reset(&self->mid);
    sab_biquad_reset(&self->treble);
    sab_biquad_reset(&self->presence);
    sab_biquad_reset(&self->cab_hpf);
    sab_biquad_reset(&self->cab_lpf);
    sab_biquad_reset(&self->fizz_notch);

    // fixed cab / safety filters
    sab_biquad_set_hpf(&self->cab_hpf, 48000, 80.0f, 0.707f);
    sab_biquad_set_lpf(&self->cab_lpf, 48000, 6200.0f, 0.707f);
    sab_biquad_set_peak(&self->fizz_notch, 48000, 4200.0f, 2.5f, -5.0f);
    sab_biquad_set_lpf(&self->input_lpf_16k, 48000, 16000.0f, 0.707f);
};

static float32_t limit(float32_t in, float32_t min, float32_t max){
	if (in>max){
		in = max;
	}else if(in<-min){
		in = -min;
	}
	return in;
}


// Process Function for SAB_overdrive_tst
float32_t SAB_overdrive_process( SAB_overdrive_tst* self, float x){
    // self->gain_f32 		= conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 10);
	// self->tone_f32		= conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,700, 1800);
    // self->volume_f32    = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 4);
    self->gain = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8,0, 1);
    self->tight = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0, 1);
    self->bass_knob = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8,0, 1);
    self->mid_knob = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,0, 1);
    self->treble_knob = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8,0, 1);
    self->presence_knob = conv_raw_to_param_value(self->intercom_parameters_aun[5].value_u8,0, 1);
    self->volume = conv_raw_to_param_value(self->intercom_parameters_aun[6].value_u8,0, 1);

    sab_beod_update(self);
    // float32_t Q_f32 = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8,0.2, 3);
    // float32_t output_f32;
    // // Clip
    // output_f32 = limit(input_f32*self->gain_f32, self->ths_f32,self->ths_f32);
    // // LPF
    // calculate_biquad_coeffs((float32_t*)&self->biquad_filter_coeffs_af32,LPF, self->tone_f32, Q_f32, 48000);
    // arm_biquad_cascade_df2T_f32(&self->biquad_filter, &output_f32, &self->biquad_filter_output_f32, 1);
    // output_f32 = (self->biquad_filter_output_f32)*self->volume_f32;
    // return output_f32;
    x = x/563647235.0;
    // input protection
//    if (x > 1.0f)  x = 1.0f;
//    if (x < -1.0f) x = -1.0f;
    x = sab_biquad_process(&self->input_lpf_16k, x);

    // input trim
    // x *= 0.8f;
    x = sab_tl072_stage(x, 2.0f);
    // // tight / bass control before distortion
    x = sab_biquad_process(&self->tight_hpf, x);
    x = sab_biquad_process(&self->pre_lpf, x);

    // // gain mapping
    float drive1 = 2.0f + self->gain * 18.0f;
    float drive2 = 1.5f + self->gain * 12.0f;

    // // TL072 gain stage 1 + red LED clip
    x = sab_tl072_stage(x, drive1);
    x = sab_red_led_clip(x);
    x = sab_biquad_process(&self->stage1_lpf, x);

    // // interstage shaping
    x = sab_biquad_process(&self->inter_hpf, x);
    x = sab_biquad_process(&self->inter_mid, x);

    // // // TL072 gain stage 2 + red LED clip
    // x = sab_tl072_stage(x, drive2);
    // x = sab_red_led_clip(x);


    x = -sab_tl072_stage(x, 4.0f);
    x = sab_dual_red_led_clip(x);
    x = sab_biquad_process(&self->stage2_lpf, x);
    // x = -sab_tl072_stage(x, 1.0f);

    // // // sag / analog feel
    x = sab_sag_process(self, x);

    // // // tone stack

    x = sab_biquad_process(&self->mid, x);

    x = sab_biquad_process(&self->presence, x);
    x = sab_red_led_clip(x);
    x = sab_biquad_process(&self->treble, x);
    x = sab_biquad_process(&self->bass, x);

    // // // simple cab sim
    x = sab_biquad_process(&self->cab_hpf, x);
    x = sab_biquad_process(&self->fizz_notch, x);
    x = sab_biquad_process(&self->cab_lpf, x);

    // // output level
    x *= self->volume * 1.8f;

    // // final safety limiter
    x = tanhf(x);

    return x*563647235.0;
};

