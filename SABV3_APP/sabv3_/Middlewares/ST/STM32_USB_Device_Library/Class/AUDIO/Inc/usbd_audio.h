/**
  ******************************************************************************
  * @file    usbd_audio.h
  * @author  MCD Application Team (adapted by Nous Research)
  * @brief   USB Audio Class 1.0 — capture (ISO IN) for SABv3.
  *
  *  This header defines the UAC capture class interface.  It replaces the
  *  original playback-oriented header shipped with STM32CubeH7.
  *
  *  Capture topology:
  *    Microphone (0x0201) -> USB Streaming (0x0101) -> PC
  *    Mono, 48 kHz, packed 24-bit, 144-byte ISO packets (48 samples per 1 ms).
  ******************************************************************************
  */

#ifndef __USBD_AUDIO_H
#define __USBD_AUDIO_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbd_def.h"
#include "usbd_core.h"

/* ---- Configuration ------------------------------------------------------- */

/* ISO IN endpoint address for capture: EP3 IN (0x83). Same address in
   audio-only and composite (CDC+UAC) layouts. */
#define AUDIO_IN_EP                          0x83U     /* ISO IN – capture data */

/* USB audio streaming configuration: 48000 Hz mono 24-bit, one ISO IN EP.
   Frame payload = (48000 / 1000) samples x 1 ch x 3 bytes = 144 bytes per frame. */
#define AUDIO_IN_PACKET                      ((uint32_t)144U)
#define AUDIO_IN_MAX_PACKET_SIZE             512U

/* Standalone UAC1 capture configuration descriptor, including AC + AS. */
#define USBD_AUDIO_CONFIG_DESC_SIZ           100U

typedef struct
{
  uint32_t            freq;
  uint32_t            volume;
  uint32_t            muted;
} USBD_AUDIO_CtrlStateTypeDef;

typedef struct
{
  USBD_AUDIO_CtrlStateTypeDef cur;       /* Current audio control state    */
  USBD_AUDIO_CtrlStateTypeDef prev;      /* Previous audio control state   */
  uint32_t              freq;            /* Audio frequency                */
  uint32_t              volume;          /* Audio volume                   */
  uint32_t              muted;           /* Audio muted flag               */
  /* Capture ring buffer: filled by board's audio-rate context, drained by ISO IN.
     Stores packed 24-bit mono samples from the guitar ADC input. */
  uint8_t               *buffer;         /* Audio data buffer (capture)    */
  uint32_t              buffer_read;     /* Buffer read pointer (bytes)    */
  uint32_t              buffer_write;    /* Buffer write pointer (bytes)   */
  uint32_t              buffer_max;      /* Buffer max size (bytes)        */
  /* Add-drop-sample sync counters for capture:
     out = samples drained by USB ISO IN, in = samples produced by board. */
  uint32_t              out;             /* Samples consumed by USB        */
  uint32_t              in;              /* Samples produced by board      */
  uint32_t              sync;            /* Add-drop-sample sync counter   */
  /* Feature-unit control (GET_CUR / SET_CUR) */
  struct
  {
    uint8_t cmd;                         /* pending request code             */
    uint8_t unit;                        /* target feature unit id           */
    uint8_t len;                         /* payload length                   */
    uint8_t data[USB_MAX_EP0_SIZE];      /* payload buffer                   */
  } control;
  uint8_t               alt_setting;     /* alternate interface setting      */
} USBD_AUDIO_HandleTypeDef;

typedef struct _USBD_AUDIO_Itf
{
  int32_t (*AudioInit       ) (uint32_t dev, uint32_t freq, uint32_t volume);
  int32_t (*AudioDeInit     ) (uint32_t dev);
  int32_t (*AudioCmd        ) (uint32_t dev, uint8_t cmd, uint8_t state);
  int32_t (*AudioVolume     ) (uint32_t dev, uint32_t volume);
  int32_t (*AudioMute       ) (uint32_t dev, uint8_t muted);
  int32_t (*PeriodicTC      ) (uint32_t dev);
  int32_t (*GetState        ) (uint32_t dev);
} USBD_AUDIO_ItfTypeDef;

/* External class object (defined in usbd_audio.c) */
extern USBD_ClassTypeDef USBD_AUDIO;

/* Bind the application audio fops into the UAC class.  Call once at startup
   (usb_device.c / main.c).  Mirrors USBD_CDC_RegisterInterface(). */
uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                  USBD_AUDIO_ItfTypeDef *fop);

/* Class API */
uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
uint8_t USBD_AUDIO_EP0_TxSent(USBD_HandleTypeDef *pdev);
uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev);
uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev);
uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
uint8_t USBD_AUDIO_IsoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);

uint8_t *USBD_AUDIO_GetHSCfgDesc(uint16_t *length);
uint8_t *USBD_AUDIO_GetFSCfgDesc(uint16_t *length);
uint8_t *USBD_AUDIO_GetOtherSpeedCfgDesc(uint16_t *length);
uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length);

/* Called by composite builder to query packet size for ISO EP allocation. */
uint32_t USBD_AUDIO_GetEpPcktSze(USBD_HandleTypeDef *pdev, uint8_t If, uint8_t Ep);

/* SOF primes the first ISO IN frame; DataIn queues subsequent frames. */

/* Push one signed, right-aligned 24-bit mono sample into the UAC ring buffer.
   Call at 48 kHz from the DSP context (after reading the guitar ADC). */
void USBD_AUDIO_PushSample(int32_t sample);

#ifdef __cplusplus
}
#endif

#endif /* __USBD_AUDIO_H */
