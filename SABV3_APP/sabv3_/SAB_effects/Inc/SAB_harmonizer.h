#ifndef SAB_HARMONIZER_H
#define SAB_HARMONIZER_H

#include "sab_intercom.h"
#include "SAB_harmonizer_dsp.h"

/* Common effect prefix must remain layout-compatible with GuitarEffect. */
typedef struct {
    void (*init)(void *);
    float (*process)(void *, float);
    void (*delete)(void *);
    fx_data_tst intercom_fx_data;
    sab_fx_param_tun intercom_parameters_aun[NUM_OF_MAX_PARAMS];
    SAB_harmonizer_dsp_tst dsp;
    uint8_t last_controls[6];
} SAB_harmonizer_tst;

void SAB_harmonizer_init(SAB_harmonizer_tst *self);
float SAB_harmonizer_process(SAB_harmonizer_tst *self, float input);
void SAB_harmonizer_delete(SAB_harmonizer_tst *self);

#endif
