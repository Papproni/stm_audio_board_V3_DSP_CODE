#include "SAB_harmonizer.h"
#include <string.h>

static void update_controls(SAB_harmonizer_tst *self)
{
    uint8_t raw[6];
    for (unsigned i = 0; i < 6; ++i) {
        raw[i] = self->intercom_parameters_aun[i].value_u8;
    }
    memcpy(self->last_controls, raw, sizeof(raw));
    SAB_harmonizer_dsp_configure(&self->dsp,
        ((unsigned)raw[0] * 7U) >> 8, raw[1] != 0U, raw[2] != 0U,
        ((unsigned)raw[5] * 11U) >> 8,
        (float)raw[3] / 255.0f, (float)raw[4] * (2.0f / 255.0f));
}

void SAB_harmonizer_init(SAB_harmonizer_tst *self)
{
    strcpy(self->intercom_fx_data.name, "Harmonizr");
    self->intercom_fx_data.color[0] = 80;
    self->intercom_fx_data.color[1] = 180;
    self->intercom_fx_data.color[2] = 255;
    self->intercom_fx_data.fx_state_en = FX_STATE_OFF;
    add_parameter(&self->intercom_parameters_aun[0], "NOTE", PARAM_TYPE_POT, 0);
    add_parameter(&self->intercom_parameters_aun[1], "KEY", PARAM_TYPE_BTN, 0);
    add_parameter(&self->intercom_parameters_aun[2], "MIN", PARAM_TYPE_BTN, 0);
    add_parameter(&self->intercom_parameters_aun[3], "MIX", PARAM_TYPE_POT, 128);
    add_parameter(&self->intercom_parameters_aun[4], "VOL", PARAM_TYPE_POT, 128);
    add_parameter(&self->intercom_parameters_aun[5], "PTCH", PARAM_TYPE_POT, 80);
    add_parameter(&self->intercom_parameters_aun[6], "NONE", PARAM_TYPE_UNUSED, 0);
    add_parameter(&self->intercom_parameters_aun[7], "NONE", PARAM_TYPE_UNUSED, 0);
    add_parameter(&self->intercom_parameters_aun[8], "NONE", PARAM_TYPE_UNUSED, 0);
    add_parameter(&self->intercom_parameters_aun[9], "NONE", PARAM_TYPE_UNUSED, 0);
    add_parameter(&self->intercom_parameters_aun[10], "NONE", PARAM_TYPE_UNUSED, 0);
    add_parameter(&self->intercom_parameters_aun[11], "NONE", PARAM_TYPE_UNUSED, 0);
    SAB_harmonizer_dsp_init(&self->dsp);
    update_controls(self);
}

float SAB_harmonizer_process(SAB_harmonizer_tst *self, float input)
{
    for (unsigned i = 0; i < 6; ++i) {
        if (self->last_controls[i] != self->intercom_parameters_aun[i].value_u8) {
            update_controls(self);
            break;
        }
    }
    /* The manager supplies signed 32-bit codec samples as floats, not +/-1. */
    return SAB_harmonizer_dsp_process(&self->dsp, input * (1.0f / 2147483648.0f))
        * 2147483648.0f;
}

void SAB_harmonizer_delete(SAB_harmonizer_tst *self)
{
    /* All buffers belong to the effect instance; the manager frees it. */
    (void)self;
}
