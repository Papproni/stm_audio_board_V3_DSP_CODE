/* Shared scalar biquad and CMSIS-compatible coefficient generation. */
#ifndef SAB_BIQUAD_H
#define SAB_BIQUAD_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float b0, b1, b2;
    float a1, a2;
    float z1, z2;
} SAB_biquad_tst;

typedef enum {
    SAB_BIQUAD_LPF,
    SAB_BIQUAD_HPF,
    SAB_BIQUAD_BANDPASS,
    SAB_BIQUAD_NOTCH,
    SAB_BIQUAD_PEAK,
    SAB_BIQUAD_LOWSHELF,
    SAB_BIQUAD_HIGHSHELF
} SAB_biquad_type_ten;

/* Valid inputs: fs > 0, 0 < fc < fs/2, q > 0, finite gain_db.
 * Output is normalized CMSIS plus-feedback {b0,b1,b2,-a1,-a2}.
 * BANDPASS has constant 0 dB peak gain. Shelf q controls alpha=sine/(2q).
 */
void SAB_biquad_coeffs(float out[5], SAB_biquad_type_ten type,
                       float fs, float fc, float q, float gain_db);

/* Compatibility entry point: original mixed-double 3.14 omega arithmetic.
 * Intended for the original LPF/HPF/BANDPASS/NOTCH generated filters.
 */
void SAB_biquad_coeffs_legacy(float out[5], SAB_biquad_type_ten type,
                              float fs, float fc, float q);

/* Explicit compatibility: preserves octave's historical b1 precedence bug
 * and mixed-double normalization. Do not replace with the ordinary LPF.
 */
void SAB_biquad_octave_sub_coeffs(float out[5]);
extern const float SAB_biquad_octave_hp50[5];
extern const float SAB_biquad_pitch_lp500[5];

/* Scalar DF2T uses standard subtract-feedback a1/a2. Setters preserve state;
 * shelf setters retain the overdrive's fixed Q=0.7071f.
 */
void SAB_biquad_set_lpf(SAB_biquad_tst *f, float fs, float fc, float q);
void SAB_biquad_set_hpf(SAB_biquad_tst *f, float fs, float fc, float q);
void SAB_biquad_set_peak(SAB_biquad_tst *f, float fs, float fc, float q,
                          float gain_db);
void SAB_biquad_set_lowshelf(SAB_biquad_tst *f, float fs, float fc,
                              float gain_db);
void SAB_biquad_set_highshelf(SAB_biquad_tst *f, float fs, float fc,
                               float gain_db);

static inline float SAB_biquad_process(SAB_biquad_tst *f, float x)
{
    const float y = f->b0 * x + f->z1;
    f->z1 = f->b1 * x - f->a1 * y + f->z2;
    f->z2 = f->b2 * x - f->a2 * y;
    return y;
}

static inline void SAB_biquad_reset(SAB_biquad_tst *f)
{
    f->z1 = 0.0f;
    f->z2 = 0.0f;
}

#ifdef __cplusplus
}
#endif
#endif
