/**
  ******************************************************************************
  * @file    usbd_audio_if.h
  * @author  MCD Application Team
  * @brief   Header for the SABv3 USB-audio interface glue (usbd_audio_if.c).
  *          This layer bridges the UAC class to the effects engine
  *          (effects_io_port) instead of the ST BSP_AUDIO_OUT driver.
  ******************************************************************************
  */

#ifndef __USBD_AUDIO_IF_H
#define __USBD_AUDIO_IF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "usbd_audio.h"

/* Application-provided audio fops for capture (board -> PC). */
extern USBD_AUDIO_ItfTypeDef USBD_AUDIO_fops;

/* One post-effects sample from the DSP's signed 32-bit path. */
void USBD_AudioIF_CaptureTick(int32_t sample);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_AUDIO_IF_H */
