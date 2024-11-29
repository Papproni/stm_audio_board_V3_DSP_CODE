/*THIS FILE IS GENERATED! DO NOT MODIFY!
Generated on: 2024.11.28. */

#ifndef SAB_FX_MANAGER_H
#define SAB_FX_MANAGER_H


#include "stdint.h"
#include "sab_intercom.h"

// Generated effect libs
#include "SAB_delay.h"
#include "SAB_overdrive.h"
#include "SAB_distortion.h"
//#include "SAB_octave.h"
#include "SAB_flanger.h"
#include "SAB_chorus.h"
#include "SAB_boost.h"
#include "SAB_pitchshift.h"
#include "SAB_fuzz.h"
#include "SAB_tremolo.h"
#include "SAB_equalizer.h"
#include "SAB_CUSTOM.h"

#include "SAB_custom_fx.h"
#include "guitar_effect_octave.h"




// Enum representing different effect types
typedef enum {
    DELAY,
    OVERDRIVE,
    DISTORTION,
    OCTAVE,
    FLANGER,
    CHORUS,
    BOOST,
    PITCHSHIFT,
    FUZZ,
    TREMOLO,
    EQUALIZER,
    CUSTOM,
    CUSTOM_FX
} EffectType;

// Struct for a guitar effect with function pointers
typedef struct {
    void (*init)(void*);           // Function pointer to initialize the effect
    int (*process)(void*);         // Function pointer to process the effect
    void (*delete)(void*);
} GuitarEffect;

// Manager structure: fx_manager
typedef struct {
	GuitarEffect*   fx_instances[12];
    EffectType      fx_types_chain[12];
    char *          fx_chain_names[12];
    
} SAB_fx_manager_tst;




// INIT for SAB_fx_manager_tst
void SAB_fx_manager_init( SAB_fx_manager_tst* self, sab_intercom_tst* intercom_pst);

// INIT for SAB_fx_manager_tst
void SAB_fx_manager_deinit( SAB_fx_manager_tst* self);

// Process Function for SAB_fx_manager_tst
int SAB_fx_manager_process( SAB_fx_manager_tst* self, float input_f32, float output_f32);

void init_effect_chain(GuitarEffect** chain, EffectType* fx_chain, int chain_length);

void process_effect_chain(GuitarEffect** chain, int chain_length);

void cleanup_effect_chain(GuitarEffect** chain, int chain_length);

#endif
