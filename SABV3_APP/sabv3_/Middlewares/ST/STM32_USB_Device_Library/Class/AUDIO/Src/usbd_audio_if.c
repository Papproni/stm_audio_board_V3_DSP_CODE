/**
  ******************************************************************************
  * @file    usbd_audio_if.c
  * @author  Nous Research (adapted from MCD Application Team)
  * @brief   USB Audio Class capture interface glue for SABv3.
  *
  *  Bridges the UAC class driver to the effects engine.
  *  Capture direction: guitar ADC input -> USBD_AUDIO_PushSample() -> ring ->
  *  DataIn() -> ISO IN endpoint 0x83 -> PC.
  ******************************************************************************
  */

#include "usbd_audio.h"
#include "usbd_audio_if.h"


/* -------------------------------------------------------------------------- */
/* Application-provided fops for capture                                      */
/* -------------------------------------------------------------------------- */

static int32_t Capture_Init(uint32_t dev, uint32_t freq, uint32_t volume)
{
  (void)dev;
  (void)freq;
  (void)volume;
  return 0;  /* SAI/DMA already running from main.c */
}

static int32_t Capture_DeInit(uint32_t dev)
{
  (void)dev;
  return 0;
}

static int32_t Capture_Cmd(uint32_t dev, uint8_t cmd, uint8_t state)
{
  (void)dev;
  (void)cmd;
  (void)state;
  return 0;
}

static int32_t Capture_Volume(uint32_t dev, uint32_t volume)
{
  (void)dev;
  (void)volume;
  return 0;
}

static int32_t Capture_Mute(uint32_t dev, uint8_t muted)
{
  (void)dev;
  (void)muted;
  return 0;
}

static int32_t Capture_PeriodicTC(uint32_t dev)
{
  (void)dev;
  return 0;
}

static int32_t Capture_GetState(uint32_t dev)
{
  (void)dev;
  return 0;
}

USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops =
{
  Capture_Init,
  Capture_DeInit,
  Capture_Cmd,
  Capture_Volume,
  Capture_Mute,
  Capture_PeriodicTC,
  Capture_GetState
};

/* -------------------------------------------------------------------------- */
/* USBD_AudioIF_CaptureTick — call at 48 kHz after the effects chain runs     */
/* Reads one left-aligned int32 sample, converts to packed PCM24 mono.       */
/* -------------------------------------------------------------------------- */
void USBD_AudioIF_CaptureTick(int32_t sample)
{
  USBD_AUDIO_PushSample(sample >> 8);
}
