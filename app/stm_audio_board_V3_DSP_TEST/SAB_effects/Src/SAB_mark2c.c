/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.29. */

#include "SAB_mark2c.h"
#include <math.h>
#include "arm_math.h"

#define MARK2C_FS 48000.0f
#define MARK2C_PI 3.14159265358979323846f

void SAB_mark2c_delete( SAB_mark2c_tst* self){
    // TODO
}

static inline float32_t dB_to_linear(float32_t db)
{
    return powf(10.0f, db / 20.0f);
}

/* RBJ peaking EQ biquad, CMSIS format: {b0,b1,b2,a1,a2} with a0=1 */
static void make_peaking_coeffs(float32_t *c,
                                float32_t freq,
                                float32_t Q,
                                float32_t gain_db)
{
    float32_t A   = powf(10.0f, gain_db / 40.0f);
    float32_t w0  = 2.0f * MARK2C_PI * freq / MARK2C_FS;
    float32_t cs  = cosf(w0);
    float32_t sn  = sinf(w0);
    float32_t alpha = sn / (2.0f * Q);

    float32_t b0 = 1.0f + alpha * A;
    float32_t b1 = -2.0f * cs;
    float32_t b2 = 1.0f - alpha * A;
    float32_t a0 = 1.0f + alpha / A;
    float32_t a1 = -2.0f * cs;
    float32_t a2 = 1.0f - alpha / A;

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    c[0] = b0;
    c[1] = b1;
    c[2] = b2;
    c[3] = -a1;   // CMSIS expects -a1, -a2
    c[4] = -a2;
}

/* RBJ high-shelf */
static void make_highshelf_coeffs(float32_t *c,
                                  float32_t freq,
                                  float32_t Q,
                                  float32_t gain_db)
{
    float32_t A   = powf(10.0f, gain_db / 40.0f);
    float32_t w0  = 2.0f * MARK2C_PI * freq / MARK2C_FS;
    float32_t cs  = cosf(w0);
    float32_t sn  = sinf(w0);
    float32_t alpha = sn / (2.0f * Q);
    float32_t sqrtA = sqrtf(A);

    float32_t b0 =    A * ((A + 1.0f) + (A - 1.0f) * cs + 2.0f * sqrtA * alpha);
    float32_t b1 = -2*A * ((A - 1.0f) + (A + 1.0f) * cs);
    float32_t b2 =    A * ((A + 1.0f) + (A - 1.0f) * cs - 2.0f * sqrtA * alpha);
    float32_t a0 =        (A + 1.0f) - (A - 1.0f) * cs + 2.0f * sqrtA * alpha;
    float32_t a1 =  2.0f*((A - 1.0f) - (A + 1.0f) * cs);
    float32_t a2 =        (A + 1.0f) - (A - 1.0f) * cs - 2.0f * sqrtA * alpha;

    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;

    c[0] = b0;
    c[1] = b1;
    c[2] = b2;
    c[3] = -a1;
    c[4] = -a2;
}

static inline float32_t fast_tanh_f32(float32_t x)
{
    float32_t x2 = x * x;
    return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}
// Update tone stack coefficients if any of TREB/MID/BASS raw changed
static void mark2c_update_tonestack_coeffs(SAB_mark2c_tst* self)
{
    uint8_t treb_raw = self->intercom_parameters_aun[2].value_u8;
    uint8_t mid_raw  = self->intercom_parameters_aun[1].value_u8;
    uint8_t bass_raw = self->intercom_parameters_aun[0].value_u8;

    if (treb_raw == self->last_treb_raw_u8 &&
        mid_raw  == self->last_mid_raw_u8  &&
        bass_raw == self->last_bass_raw_u8) {
        return; // no change
    }

    self->last_treb_raw_u8 = treb_raw;
    self->last_mid_raw_u8  = mid_raw;
    self->last_bass_raw_u8 = bass_raw;

    // BASS: low shelf ~100 Hz
    make_peaking_coeffs(&self->tonestack_coeffs_af32[0],
                        100.0f, 0.7f, self->bass_db_f32);

    // MID: peaking ~750 Hz
    make_peaking_coeffs(&self->tonestack_coeffs_af32[5],
                        750.0f, 1.0f, self->mid_db_f32);

    // TREB: high shelf ~3.5 kHz (approximated as peaking with wide Q)
    make_peaking_coeffs(&self->tonestack_coeffs_af32[10],
                        3500.0f, 0.5f, self->treb_db_f32);
}

// Update GEQ biquads if any slider changed
static void mark2c_update_geq_coeffs(SAB_mark2c_tst* self)
{
    uint8_t raw80   = self->intercom_parameters_aun[6].value_u8;
    uint8_t raw240  = self->intercom_parameters_aun[7].value_u8;
    uint8_t raw750  = self->intercom_parameters_aun[8].value_u8;
    uint8_t raw2200 = self->intercom_parameters_aun[9].value_u8;
    uint8_t raw6600 = self->intercom_parameters_aun[10].value_u8;

    if (raw80   == self->last_geq_raw_u8[0] &&
        raw240  == self->last_geq_raw_u8[1] &&
        raw750  == self->last_geq_raw_u8[2] &&
        raw2200 == self->last_geq_raw_u8[3] &&
        raw6600 == self->last_geq_raw_u8[4]) {
        return;
    }

    self->last_geq_raw_u8[0] = raw80;
    self->last_geq_raw_u8[1] = raw240;
    self->last_geq_raw_u8[2] = raw750;
    self->last_geq_raw_u8[3] = raw2200;
    self->last_geq_raw_u8[4] = raw6600;

    // Use Q ≈ 1.0 for all GEQ bands
    make_peaking_coeffs(&self->geq_coeffs_af32[0],
                        80.0f, 1.0f, self->geq_80_db_f32);
    make_peaking_coeffs(&self->geq_coeffs_af32[5],
                        240.0f, 1.0f, self->geq_240_db_f32);
    make_peaking_coeffs(&self->geq_coeffs_af32[10],
                        750.0f, 1.0f, self->geq_750_db_f32);
    make_peaking_coeffs(&self->geq_coeffs_af32[15],
                        2200.0f, 1.0f, self->geq_2200_db_f32);
    make_peaking_coeffs(&self->geq_coeffs_af32[20],
                        6600.0f, 1.0f, self->geq_6600_db_f32);
}

// Update presence shelf biquad if presence changed
static void mark2c_update_presence_coeffs(SAB_mark2c_tst* self)
{
    uint8_t pres_raw = self->intercom_parameters_aun[11].value_u8;
    if (pres_raw == self->last_presence_raw_u8) {
        return;
    }
    self->last_presence_raw_u8 = pres_raw;

    // Presence: high-shelf around 4 kHz
    make_highshelf_coeffs(self->presence_coeffs_af32,
                          4000.0f, 0.7f, self->presence_db_f32);
}

float32_t mark2c_hpf_process(SAB_mark2c_tst* self, float32_t x)
{
    // y[n] = a*(y[n-1] + x[n] - x[n-1])
    float32_t y = self->hpf_a_f32 * (self->hpf_y1_f32 + x - self->hpf_x1_f32);
    self->hpf_x1_f32 = x;
    self->hpf_y1_f32 = y;
    return y;
}

float32_t mark2c_tonestack_process(SAB_mark2c_tst* self, float32_t x)
{
    // Update coefficients on knob change
    mark2c_update_tonestack_coeffs(self);

    float32_t out;
    arm_biquad_cascade_df2T_f32(&self->tonestack, &x, &out, 1);
    return out;
}

float32_t mark2c_softclip(SAB_mark2c_tst* self, float32_t x)
{
    (void)self; // not used yet, but kept for future tweaks

    // Soft symmetric saturation
    // You can tweak "drive" here if you want later
    // return fast_tanh_f32(x);

     // Increase drive into saturation
    
    // Add makeup gain to compensate
    return 1.4 *  tanhf(x * 2.5f);
}

#define NORM_ADC_VAL 563647235.0f // Measured max abs value from 24-bit ADC (2^23 = 8388608)
#define ADC_24_NORM   (1.0f / NORM_ADC_VAL)  // 1 / 2^23
float32_t mark2c_asym_clip(SAB_mark2c_tst* self, float32_t x)
{
    // Lead drive controls total gain into the stage
    float32_t z = x * self->lead_drive_f32;

    // Tube-like asymmetry: small bias shift
    const float32_t bias = 0.1f;      // small: avoids DC
    const float32_t asym = 0.2f;      // asymmetry strength

    // Apply bias asymmetrically and soft saturation
    float32_t up = tanhf((z + bias) * (1.0f + asym));
    float32_t dn = tanhf((z - bias) * (1.0f - asym));

    // Blend smoothly, NEVER hard branch
    float32_t blend = 0.5f * (1.0f + (z * 2.0f));  // smooth transition
    blend = fminf(fmaxf(blend, 0.0f), 1.0f);

    return up * blend + dn * (1.0f - blend);
}
float32_t mark2c_cathode_soften(SAB_mark2c_tst* self, float32_t x)
{
    // First-order LPF at ~6 kHz:
    // y[n] = a*y[n-1] + (1-a)*x[n]
    float32_t y_lp = self->cf_a_f32 * self->cf_y1_f32
                   + (1.0f - self->cf_a_f32) * x;
    self->cf_y1_f32 = y_lp;

    // Mix dry and low-passed for "softening"
    return 0.6f * x + 0.4f * y_lp;
}

float32_t mark2c_geq_process(SAB_mark2c_tst* self, float32_t x)
{
    // Update coefficients on slider change
    mark2c_update_geq_coeffs(self);

    float32_t out;
    arm_biquad_cascade_df2T_f32(&self->geq, &x, &out, 1);
    return out;
}

float32_t mark2c_presence_shelf(SAB_mark2c_tst* self, float32_t x)
{
    // Update only if presence knob changed
    mark2c_update_presence_coeffs(self);

    float32_t out;
    arm_biquad_cascade_df2T_f32(&self->presence, &x, &out, 1);
    return out;
}

// Keeley-style compression settings
float32_t MARK2C_envelope_attack = 0.01f;    // Fast attack: ~0.5ms at 48kHz
float32_t MARK2C_envelope_release = 0.10f; // Slow release: ~20ms for sustain
float32_t MARK2C_threshold = 0.15f;          // Threshold (starts compressing around here)
float32_t MARK2C_ratio = 4.0f;               // 2.5:1 ratio (transparent, musical)
float32_t MARK2C_soft_knee = 0.1f;           // Soft knee width in dB
float32_t sound_sample_f32;
// Keeley-style: transparent, smooth, adds sustain
float32_t mark2c_envelope_compress(SAB_mark2c_tst* self, float32_t x)
{   
    sound_sample_f32 = x;
    float32_t abs_x = fabsf(x);
    
    // Envelope tracking with separate attack/release
    if (abs_x > self->comp_env_f32) {
        // Attack phase: fast response to transients
        self->comp_env_f32 += MARK2C_envelope_attack * (abs_x - self->comp_env_f32);
    } else {
        // Release phase: slow recovery (creates sustain)
        self->comp_env_f32 += MARK2C_envelope_release * (abs_x - self->comp_env_f32);
    }
    
    // Soft-knee compression (smooth transition into limiting)
    float32_t knee_start = MARK2C_threshold - MARK2C_soft_knee;
    float32_t knee_end = MARK2C_threshold + MARK2C_soft_knee;
    
    float32_t gain_reduction = 1.0f;  // Default: no compression
    
    if (self->comp_env_f32 > knee_start) {
        if (self->comp_env_f32 < knee_end) {
            // Inside soft-knee zone: gradual transition
            float32_t knee_factor = (self->comp_env_f32 - knee_start) / (2.0f * MARK2C_soft_knee);
            float32_t soft_ratio = 1.0f + knee_factor * (MARK2C_ratio - 1.0f);
            float32_t excess = self->comp_env_f32 - knee_start;
            gain_reduction = 1.0f / (1.0f + (soft_ratio - 1.0f) * excess / MARK2C_soft_knee);
        } else {
            // Past knee: full compression ratio
            float32_t excess = self->comp_env_f32 - MARK2C_threshold;
            gain_reduction = 1.0f / (1.0f + (MARK2C_ratio - 1.0f) * excess / MARK2C_threshold);
        }
    }
    
    // Makeup gain: restore perceived level (Keeley style = subtle, musical)
    return x * gain_reduction * 1.35f;
}



// Process Function for SAB_mark2c_tst
void SAB_mark2c_init( SAB_mark2c_tst* self){
    strcpy(self->intercom_fx_data.name, "MarkIIc");
	self->intercom_fx_data.color[0] = 0; 	// R
	self->intercom_fx_data.color[1] = 255;	// G
	self->intercom_fx_data.color[2] = 0;	// B
	self->intercom_fx_data.fx_state_en = FX_STATE_OFF;

	// PARAMS:
    // PAGE1
    
    add_parameter(&self->intercom_parameters_aun[0],  "BASS", PARAM_TYPE_POT, 69);   // Bass
    add_parameter(&self->intercom_parameters_aun[1],  "MIDL", PARAM_TYPE_POT, 69);   // Middle
    add_parameter(&self->intercom_parameters_aun[2],  "TREB", PARAM_TYPE_POT, 69);   // Treble
    add_parameter(&self->intercom_parameters_aun[3],  "PRE", PARAM_TYPE_POT, 69);   // Input gain
    add_parameter(&self->intercom_parameters_aun[4],  "LDRV", PARAM_TYPE_POT, 69);   // Lead Drive
    add_parameter(&self->intercom_parameters_aun[5], "MAST", PARAM_TYPE_POT, 69);   // Master Volume
    // PAGE2
    add_parameter(&self->intercom_parameters_aun[6],  "80",  PARAM_TYPE_POT, 69);   // GEQ 80 Hz
    add_parameter(&self->intercom_parameters_aun[7],  "240", PARAM_TYPE_POT, 69);   // GEQ 240 Hz
    add_parameter(&self->intercom_parameters_aun[8],  "750", PARAM_TYPE_POT, 69);   // GEQ 750 Hz
    add_parameter(&self->intercom_parameters_aun[9],  "2k2",  PARAM_TYPE_POT, 69);   // GEQ 2200 Hz
    add_parameter(&self->intercom_parameters_aun[10],  "6k6",  PARAM_TYPE_POT, 69);   // GEQ 6600 Hz
    add_parameter(&self->intercom_parameters_aun[11], "PRES", PARAM_TYPE_POT, 69);   // Presence
    

     // ----- HPF at ~90 Hz -----
    float32_t fc_hp = 90.0f;
    self->hpf_a_f32 = expf(-2.0f * MARK2C_PI * fc_hp / MARK2C_FS);
    self->hpf_x1_f32 = 0.0f;
    self->hpf_y1_f32 = 0.0f;

    // ----- Tone stack: 3-band -----
    memset(self->tonestack_state_af32, 0, sizeof(self->tonestack_state_af32));
    arm_biquad_cascade_df2T_init_f32(&self->tonestack,
                                     3,
                                     self->tonestack_coeffs_af32,
                                     self->tonestack_state_af32);
    self->last_treb_raw_u8 = 255;
    self->last_mid_raw_u8  = 255;
    self->last_bass_raw_u8 = 255;

    // ----- GEQ: 5-band -----
    memset(self->geq_state_af32, 0, sizeof(self->geq_state_af32));
    arm_biquad_cascade_df2T_init_f32(&self->geq,
                                     5,
                                     self->geq_coeffs_af32,
                                     self->geq_state_af32);
    for (int i = 0; i < 5; i++) {
        self->last_geq_raw_u8[i] = 255;
    }

    // ----- Presence -----
    memset(self->presence_state_af32, 0, sizeof(self->presence_state_af32));
    arm_biquad_cascade_df2T_init_f32(&self->presence,
                                     1,
                                     self->presence_coeffs_af32,
                                     self->presence_state_af32);
    self->last_presence_raw_u8 = 255;

    // ----- Cathode LPF (~6 kHz) -----
    float32_t fc_cf = 6000.0f;
    self->cf_a_f32  = expf(-2.0f * MARK2C_PI * fc_cf / MARK2C_FS);
    self->cf_y1_f32 = 0.0f;


    self->comp_env_f32 = 0.0f;
    self->comp_threshold_f32 = 0.4f;
    self->comp_ratio_f32 = 4.0f;

};

// Process Function for SAB_mark2c_tst
float32_t SAB_mark2c_process( SAB_mark2c_tst* self, float input_f32){

    
    
    self->bass_db_f32      = conv_raw_to_param_value(self->intercom_parameters_aun[0].value_u8, -12.0f, 12.0f);
    self->mid_db_f32       = conv_raw_to_param_value(self->intercom_parameters_aun[1].value_u8, -12.0f, 12.0f);
    self->treb_db_f32      = conv_raw_to_param_value(self->intercom_parameters_aun[2].value_u8, -12.0f, 12.0f);
    self->gain_pre_f32     = conv_raw_to_param_value(self->intercom_parameters_aun[3].value_u8,  1.0f, 10.0f);
    self->lead_drive_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[4].value_u8,  1.0f, 12.0f);
    self->master_f32       = conv_raw_to_param_value(self->intercom_parameters_aun[5].value_u8,  0.0f,   2.0f);

    self->geq_80_db_f32    = conv_raw_to_param_value(self->intercom_parameters_aun[6].value_u8,  -12.0f, 12.0f);
    self->geq_240_db_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[7].value_u8,  -12.0f, 12.0f);
    self->geq_750_db_f32   = conv_raw_to_param_value(self->intercom_parameters_aun[8].value_u8,  -18.0f, 12.0f);
    self->geq_2200_db_f32  = conv_raw_to_param_value(self->intercom_parameters_aun[9].value_u8,  -12.0f, 12.0f);
    self->geq_6600_db_f32  = conv_raw_to_param_value(self->intercom_parameters_aun[10].value_u8,  -12.0f, 12.0f);
    self->presence_db_f32  = conv_raw_to_param_value(self->intercom_parameters_aun[11].value_u8, -12.0f,  12.0f);
    

    float32_t x = input_f32* ADC_24_NORM; // Normalize 24-bit input to -1.0 to +1.0
    x = mark2c_hpf_process(self, x);
    // x = mark2c_envelope_compress(self, x);      // ADD THIS
    x = mark2c_tonestack_process(self, x);
    x = mark2c_softclip(self, x * self->gain_pre_f32);
    x = mark2c_asym_clip(self, x);
    x = mark2c_cathode_soften(self, x);
    x = mark2c_geq_process(self, x);
    x = mark2c_presence_shelf(self, x);
    x *= self->master_f32;

    return x*NORM_ADC_VAL;
};

