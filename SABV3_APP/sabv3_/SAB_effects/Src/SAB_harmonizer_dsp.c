#include "SAB_harmonizer_dsp.h"
#include <math.h>
#include <string.h>

/* Optimized implementation, informed by:
 * A. de Cheveigne and H. Kawahara, "YIN, a fundamental frequency estimator
 * for speech and music", JASA 111(4), 2002, DOI 10.1121/1.1458024.
 * https://www.ee.columbia.edu/~dpwe/e6820/papers/deChevK02-yin.pdf
 * Uses squared differences, CMND, first confident trough and interpolation;
 * not the paper's full multi-estimate best-local-estimate refinement.
 * Delay-line modulation background: DAFX 2nd ed., Chapter 6, Dutilleux et al.
 * https://www.dafx.de/DAFX_Book_Page_2nd_edition/chapter6.html
 * No reference source code copied.
 *
 * OPTIMIZATIONS FOR FASTER TRACKING:
 * Detector: four-pole 1200 Hz LP -> /12 -> 4 kHz, 128 samples frozen (was 160);
 * 64 comparison samples (was 96), lags 1..64. Hop 32 detector samples (8 ms, was 24 ms).
 * Snapshot is copied eight floats/call over 16 callbacks (was 20). 
 * Difference stage: 64*64 MACs, eight/call (512 callbacks, was 768). 
 * Worst analysis <=660 callbacks (was 913), reducing CPU load.
 * Single confident frame (min < 0.15) establishes a new MIDI note; 0.65-semitone 
 * hysteresis prevents flutter without needing multi-frame confirmation.
 * Fixed cold analysis schedule: first estimate about 40 ms, voiced about
 * 45-50 ms for stationary tones (was 58 ms / 82 ms). Unvoice timeout reduced
 * to 50 ms (2400 callbacks) to drop stale notes faster. Dry path has zero 
 * lookahead. Wet delay is variable 16..~1724 samples (<36 ms).
 */

static const float semitone_ratios[49] = {
    0.25f,0.2648657736f,0.2806155121f,0.2973017788f,0.3149802625f,
    0.3337099635f,0.3535533906f,0.3745767692f,0.396850263f,0.4204482076f,
    0.4454493591f,0.4719371563f,0.5f,0.5297315472f,0.5612310242f,
    0.5946035575f,0.6299605249f,0.6674199271f,0.7071067812f,0.7491535384f,
    0.793700526f,0.8408964153f,0.8908987181f,0.9438743127f,1.0f,
    1.059463094f,1.122462048f,1.189207115f,1.25992105f,1.334839854f,
    1.414213562f,1.498307077f,1.587401052f,1.681792831f,1.781797436f,
    1.887748625f,2.0f,2.118926189f,2.244924097f,2.37841423f,
    2.5198421f,2.669679708f,2.828427125f,2.996614154f,3.174802104f,
    3.363585661f,3.563594873f,3.775497251f,4.0f
};

static int floor_div(int n, int d)
{
    int q = n / d;
    return q - (n % d < 0);
}

int SAB_harmonizer_interval_semitones(int midi_note, unsigned tonic_pitch_class,
                                     unsigned minor, int interval)
{
    static const int scales[2][7] = {{0,2,4,5,7,9,11},{0,2,3,5,7,8,10}};
    const int *scale = scales[minor != 0];
    int relative = midi_note % 12 - (int)(tonic_pitch_class % 12);
    int octave = floor_div(relative, 12);
    int pc = relative - 12 * octave;
    int best = 0, distance = 100, i, steps, destination;
    /* Include next octave tonic; ascending search gives lower-note tie break. */
    for (i = 0; i <= 7; ++i) {
        int degree = i == 7 ? 12 : scale[i];
        int delta = degree - pc;
        if (delta < 0) delta = -delta;
        if (delta < distance) { distance = delta; best = i; }
    }
    if (interval > 8) interval = 8;
    if (interval < -8) interval = -8;
    steps = interval > 0 ? interval - 1 : interval < 0 ? interval + 1 : 0;
    destination = best + steps;
    return scale[destination - 7 * floor_div(destination,7)]
         + 12 * floor_div(destination,7) - (best == 7 ? 12 : scale[best]);
}

static float clamp_finite(float x, float maximum)
{
    if (!(x >= 0.0f)) return 0.0f; /* includes NaN */
    return x > maximum ? maximum : x;
}

static void update_harmonies(SAB_harmonizer_dsp_tst *s)
{
    static const int intervals[11][2] = {
        {-6,0},{-4,0},{-3,0},{3,0},{4,0},{5,0},{6,0},{7,0},
        {3,5},{3,7},{5,8}
    };
    unsigned voice;
    s->voice_count = s->pitch_index >= 8 ? 2u : 1u;
    for (voice = 0; voice < 2; ++voice) {
        int shift = SAB_harmonizer_interval_semitones(s->midi_note,
            s->tonic, s->minor, intervals[s->pitch_index][voice]);
        s->ratios[voice] = semitone_ratios[shift + 24];
    }
}

void SAB_harmonizer_dsp_configure(SAB_harmonizer_dsp_tst *s,
    unsigned note_index, unsigned sharp, unsigned minor, unsigned pitch_index,
    float mix, float volume)
{
    static const unsigned natural[7] = {0,2,4,5,7,9,11};
    if (note_index > 6) note_index = 6;
    s->tonic = (natural[note_index] + (sharp != 0)) % 12;
    s->minor = minor != 0;
    s->pitch_index = pitch_index > 10 ? 10 : pitch_index;
    s->target_mix = clamp_finite(mix, 1.0f);
    s->target_volume = clamp_finite(volume, 2.0f);
    update_harmonies(s);
}

void SAB_harmonizer_dsp_init(SAB_harmonizer_dsp_tst *s)
{
    memset(s, 0, sizeof(*s));
    SAB_biquad_set_lpf(&s->lowpass[0], 48000, 1200, 0.5411961f);
    SAB_biquad_set_lpf(&s->lowpass[1], 48000, 1200, 1.3065630f);
    s->midi_note = 60;
    s->candidate_note = -1;
    s->span = s->target_span = 1280;
    s->phase[1] = 0.25f;
    s->current_ratio[0] = s->current_ratio[1] = 1;
    s->mix = 0.5f;
    s->volume = 1;
    SAB_harmonizer_dsp_configure(s, 0, 0, 0, 3, 0.5f, 1);
}

static void unvoice(SAB_harmonizer_dsp_tst *s)
{
    s->voiced = 0;
    s->frequency_hz = 0;
    s->candidate_count = 0;
    s->candidate_note = -1;
}

static void finish_pitch(SAB_harmonizer_dsp_tst *s)
{
    unsigned lag = s->scan;
    float a = s->difference[lag-1], b = s->difference[lag];
    float c = s->difference[lag+1], denominator = a - 2*b + c;
    float offset = denominator > 1e-12f ? 0.5f*(a-c)/denominator : 0;
    float period, frequency, midi, desired_span;
    int note, periods;
    if (offset < -0.5f) offset = -0.5f;
    if (offset > 0.5f) offset = 0.5f;
    period = (float)lag + offset;
    frequency = 4000.0f / period;
    if (frequency < 64.0f || frequency > 1020.0f ||
        s->frame_energy < 0.000016f || s->envelope < 0.0005f) {
        unvoice(s);
        return;
    }
    s->valid_age = 0;
    s->frequency_hz = frequency;
    midi = 69.0f + 17.31234049f * logf(frequency / 440.0f);
    note = (int)floorf(midi + 0.5f);
    
    /* Hysteresis: prevent rapid fluttering on ambiguous boundaries */
    if (s->voiced && fabsf(midi - (float)s->midi_note) < 0.65f)
        note = s->midi_note;

    /* FAST TRACKING: Trust the first confident frame. Stage 5 is only reached 
     * if CMND minimum < 0.15f, which is a strict confidence threshold that 
     * reliably rejects noise without needing multi-frame confirmation. */
    s->voiced = 1;
    if (note != s->midi_note) { 
        s->midi_note = note; 
        update_harmonies(s); 
    }

    periods = (int)(640.0f / (12.0f * period) + 0.5f);
    if (periods < 1) periods = 1;
    desired_span = 24.0f * period * (float)periods;
    if (desired_span < 800) desired_span = 800;
    if (desired_span > 1720) desired_span = 1720;
    s->target_span = desired_span;
}

static void analyze_step(SAB_harmonizer_dsp_tst *s)
{
    unsigned n;
    s->analysis_ops_last = 0;
    switch (s->stage) {
    case 1: /* Frozen indices, incremental copy; live ring cannot catch us. */
        for (n = 0; n < 8; ++n) {
            float value = s->detector[(s->snapshot_start + s->cursor) & 255u];
            s->frame[s->cursor++] = value;
            s->frame_energy += value * value;
            ++s->analysis_ops_last;
        }
        /* OPTIMIZED: 128 samples is >2 periods of 82 Hz (lowest guitar E2) */
        if (s->cursor == 128) { 
            s->lag = 1; s->comparison = 0; s->difference_sum = 0;
            s->stage = 2;
        }
        break;
    case 2:
        for (n = 0; n < 8; ++n) {
            /* OPTIMIZED: 64 comparison samples keeps max index (63+64=127) 
             * safely within the 128-sample frame boundary. */
            float d = s->frame[s->comparison] - s->frame[s->comparison+s->lag];
            s->difference_sum += d*d;
            ++s->analysis_ops_last;
            if (++s->comparison == 64) { 
                s->difference[s->lag++] = s->difference_sum;
                s->comparison = 0; s->difference_sum = 0;
                if (s->lag == 65) {
                    s->stage = 3; s->lag = 1; s->cmnd_sum = 0;
                    s->cmnd[0] = 1;
                    break;
                }
            }
        }
        break;
    case 3: /* One normalization/divide per callback, not a 64-lag burst. */
        s->cmnd_sum += s->difference[s->lag];
        s->cmnd[s->lag] = s->cmnd_sum > 1e-20f ?
            s->difference[s->lag] * (float)s->lag / s->cmnd_sum : 1;
        s->analysis_ops_last = 1;
        if (++s->lag == 65) { s->stage = 4; s->scan = 4; }
        break;
    case 4: { /* First interpolated CMND trough; sparse high-note lags need
               * sub-sample confidence too, otherwise 880 Hz becomes 440 Hz. */
        float left = s->cmnd[s->scan-1], middle = s->cmnd[s->scan];
        float right = s->cmnd[s->scan+1];
        float curvature = left - 2*middle + right;
        float minimum = curvature > 1e-12f ?
            middle - (left-right)*(left-right)/(8*curvature) : middle;
        s->analysis_ops_last = 1;
        if (minimum < 0.15f && middle <= left && middle < right) s->stage = 5;
        else if (++s->scan == 64) {
            unvoice(s); s->stage = 0; ++s->completed_frames;
        }
        break;
    }
    case 5:
        finish_pitch(s);
        s->analysis_ops_last = 1;
        ++s->completed_frames; s->stage = 0;
        break;
    default: break;
    }
    if (s->analysis_ops_last > s->analysis_ops_max)
        s->analysis_ops_max = s->analysis_ops_last;
}

static float read_delay(const SAB_harmonizer_dsp_tst *s, float delay)
{
    float position = (float)s->write - delay;
    unsigned index;
    float fraction;
    if (position < 0) position += 2048;
    index = (unsigned)position;
    fraction = position - (float)index;
    return s->audio[index & 2047u] * (1-fraction)
         + s->audio[(index+1) & 2047u] * fraction;
}

float SAB_harmonizer_dsp_process(SAB_harmonizer_dsp_tst *s, float x)
{
    float detector_value, wet = 0;
    unsigned v;
    /* Also protects state from hostile nonfinite inputs. Audio is not clipped
     * to unity: volume=2 intentionally permits a full-scale output of 2. */
    if (!isfinite(x)) x = 0;
    if (x > 4) x = 4;
    if (x < -4) x = -4;
    s->audio[s->write] = x;
    s->envelope += 0.001f * (fabsf(x) - s->envelope);
    detector_value = SAB_biquad_process(&s->lowpass[1],
        SAB_biquad_process(&s->lowpass[0], x));
    if (++s->decimator == 12) {
        s->decimator = 0;
        s->detector[s->detector_write] = detector_value;
        s->detector_write = (s->detector_write + 1) & 255u;
        if (s->detector_count < 128) ++s->detector_count; /* OPTIMIZED: was 160 */
        ++s->hop;
        /* OPTIMIZED: Trigger analysis sooner (32 hop vs 96) */
        if (s->detector_count == 128 && s->hop >= 32 && s->stage == 0) {
            s->snapshot_start = (s->detector_write + 256 - 128) & 255u;
            s->hop = 0; s->cursor = 0; s->frame_energy = 0; s->stage = 1;
        }
    }
    analyze_step(s);
    
    /* OPTIMIZED: Drop stale notes faster (50 ms vs 100 ms) to clear way for new notes */
    if (s->valid_age < 2401) ++s->valid_age;
    if (s->envelope < 0.0005f || s->valid_age > 2400) unvoice(s);
    
    s->gate += 0.004f * ((s->voiced ? 1.0f : 0.0f) - s->gate);
    s->mix += 0.002f * (s->target_mix - s->mix);
    s->volume += 0.002f * (s->target_volume - s->volume);
    s->span += 0.001f * (s->target_span - s->span);
    for (v = 0; v < 2; ++v) {
        float p, q, weight, gain_target;
        s->current_ratio[v] += 0.001f * (s->ratios[v] - s->current_ratio[v]);
        /* Opposite change in read delay makes read position advance at ratio. */
        p = s->phase[v] + (1.0f - s->current_ratio[v]) / s->span;
        if (p < 0) p += 1;
        if (p >= 1) p -= 1;
        s->phase[v] = p;
        q = p + 0.5f;
        if (q >= 1) q -= 1;
        weight = 1.0f - fabsf(2.0f*p - 1.0f);
        gain_target = v < s->voice_count ? 1.0f/(float)s->voice_count : 0;
        s->voice_gain[v] += 0.002f * (gain_target - s->voice_gain[v]);
        wet += s->voice_gain[v] * (weight * read_delay(s,16+s->span*p)
             + (1-weight) * read_delay(s,16+s->span*q));
    }
    s->write = (s->write + 1) & 2047u;
    return s->volume * ((1-s->mix)*x + s->mix*s->gate*wet);
}