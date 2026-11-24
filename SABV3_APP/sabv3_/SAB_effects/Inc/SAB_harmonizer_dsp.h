#ifndef SAB_HARMONIZER_DSP_H
#define SAB_HARMONIZER_DSP_H
#include "SAB_biquad.h"
#ifdef __cplusplus
extern "C" {
#endif

/* Fixed 48 kHz, normalized approximately [-1,1], monophonic only.
 * All buffers belong to the instance; no allocator or SDRAM pool required.
 * Controls are non-atomic: configure and process must be externally serialized.
 * Public observations: ratios are targets, midi_note is last stable source note,
 * voiced/frequency_hz clear on loss of confidence; midi_note remains historical.
 */
typedef struct {
    float frequency_hz;
    int midi_note;
    unsigned voiced;
    float ratios[2];
    unsigned voice_count;
    /* Instrumentation: max eight snapshot copies/difference MACs per callback;
     * other stages perform one scalar CMND/search/finalization step each. */
    unsigned analysis_ops_last, analysis_ops_max, completed_frames;
    float audio[2048], detector[256], frame[160], difference[65], cmnd[65];
    SAB_biquad_tst lowpass[2];
    float phase[2], current_ratio[2], voice_gain[2];
    float span, target_span, mix, target_mix, volume, target_volume;
    float gate, envelope, difference_sum, cmnd_sum, frame_energy;
    unsigned write, detector_write, decimator, detector_count, hop;
    unsigned stage, cursor, snapshot_start, lag, comparison, scan;
    unsigned tonic, minor, pitch_index, candidate_count, valid_age;
    int candidate_note;
} SAB_harmonizer_dsp_tst;

void SAB_harmonizer_dsp_init(SAB_harmonizer_dsp_tst *s);
void SAB_harmonizer_dsp_configure(SAB_harmonizer_dsp_tst *s,
    unsigned note_index, unsigned sharp, unsigned minor, unsigned pitch_index,
    float mix, float volume);
float SAB_harmonizer_dsp_process(SAB_harmonizer_dsp_tst *s, float x);

/* Signed diatonic intervals, not semitones. Nearest scale degree for chromatic
 * sources; exact ties choose LOWER degree. Return target-minus-snapped-source,
 * preserving original chromatic displacement/detune (no pitch correction).
 * Valid interval magnitudes 1..8; other values clamp, zero is unison.
 */
int SAB_harmonizer_interval_semitones(int midi_note, unsigned tonic_pitch_class,
                                     unsigned minor, int interval);
#ifdef __cplusplus
}
#endif
#endif
