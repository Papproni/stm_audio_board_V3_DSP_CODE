/**
  ******************************************************************************
  * @file    usbd_audio.c
  * @author  MCD Application Team (adapted by Nous Research)
  * @brief   USB Audio Class 1.0 capture (ISO IN) for SABv3.
  *
  *  Capture topology: Microphone -> USB Streaming -> PC
  *  Mono, 48 kHz, 24-bit; 144-byte ISO packets (48 samples x 1 ch x 3 bytes).
  ******************************************************************************
  */

#include "usbd_audio.h"
#include "usbd_ctlreq.h"

/* Capture ring buffer: 16 ISO frames of 144 bytes = 2304 bytes */
#define CAPTURE_BUF_SIZE   (AUDIO_IN_PACKET * 16U)

static volatile uint32_t fill_ptr = 0U;
static volatile uint32_t drain_ptr = 0U;
static uint8_t capture_buf[CAPTURE_BUF_SIZE];
__ALIGN_BEGIN static uint8_t capture_packet[AUDIO_IN_PACKET] __ALIGN_END;
static volatile uint8_t capture_tx_busy;
volatile uint32_t audio_sof_armed_count;
volatile uint32_t audio_in_complete_count;
volatile uint32_t audio_in_incomplete_count;
volatile uint32_t audio_samples_produced;
volatile uint32_t audio_samples_dropped;
volatile uint32_t audio_iso_underflows;

/* UAC1 microphone: IF0 AudioControl, IF1 AudioStreaming alt 0/1
 * when standalone; the composite builder maps these to IF2 and IF3.
 * Class-specific AC header covers the 12-byte input and 9-byte output terminal.
 * No feature unit or adjustable sample rate is advertised. */
__ALIGN_BEGIN static uint8_t AUDIO_FSCfgDesc[USBD_AUDIO_CONFIG_DESC_SIZ] __ALIGN_END =
{
  9, USB_DESC_TYPE_CONFIGURATION, LOBYTE(USBD_AUDIO_CONFIG_DESC_SIZ),
  HIBYTE(USBD_AUDIO_CONFIG_DESC_SIZ), 2, 1, 0, 0xC0, 50,
  /* AC interface 0 */
  9, USB_DESC_TYPE_INTERFACE, 0, 0, 0, 1, 1, 0, 0,
  /* Class-specific AC header: UAC1, 30 AC bytes, one streaming interface */
  9, 0x24, 1, 0x00, 0x01, 0x1E, 0x00, 1, 1,
  /* Input terminal 1: microphone, mono */
  12, 0x24, 2, 1, 0x01, 0x02, 0, 1, 0, 0, 0, 0,
  /* Output terminal 2: USB streaming, source terminal 1 */
  9, 0x24, 3, 2, 0x01, 0x01, 0, 1, 0,
  /* AS interface 1, alt 0 (zero bandwidth) */
  9, USB_DESC_TYPE_INTERFACE, 1, 0, 0, 1, 2, 0, 0,
  /* AS interface 1, alt 1 (ISO capture) */
  9, USB_DESC_TYPE_INTERFACE, 1, 1, 1, 1, 2, 0, 0,
  /* AS general: terminal 2, PCM */
  7, 0x24, 1, 2, 1, 1, 0,
  /* Format Type I: mono, signed packed PCM24, one discrete rate (48000 Hz) */
  11, 0x24, 2, 1, 1, 3, 24, 1, 0x80, 0xBB, 0x00,
  /* EP3 IN: asynchronous isochronous, 144 bytes per 1-ms frame */
  9, USB_DESC_TYPE_ENDPOINT, AUDIO_IN_EP, 0x05,
     LOBYTE(AUDIO_IN_PACKET), HIBYTE(AUDIO_IN_PACKET), 1, 0, 0,
  /* Class-specific ISO endpoint: no frequency or pitch control */
  7, 0x25, 1, 0, 0, 0, 0
};

static inline uint32_t ring_available(void)
{
  if (fill_ptr >= drain_ptr)
    return fill_ptr - drain_ptr;
  else
    return CAPTURE_BUF_SIZE - (drain_ptr - fill_ptr);
}
/* Class object - matches USBD_ClassTypeDef */
USBD_ClassTypeDef USBD_AUDIO =
{
  USBD_AUDIO_Init,                        /* Init */
  USBD_AUDIO_DeInit,                      /* DeInit */
  USBD_AUDIO_Setup,                       /* Setup */
  USBD_AUDIO_EP0_TxSent,                  /* EP0_TxSent */
  USBD_AUDIO_EP0_RxReady,                 /* EP0_RxReady */
  USBD_AUDIO_DataIn,                      /* DataIn */
  USBD_AUDIO_DataOut,                     /* DataOut */
  USBD_AUDIO_SOF,                         /* SOF */
  USBD_AUDIO_IsoINIncomplete,             /* IsoINIncomplete */
  USBD_AUDIO_IsoOUTIncomplete,            /* IsoOUTIncomplete */
  USBD_AUDIO_GetHSCfgDesc,                /* GetHSConfigDescriptor */
  USBD_AUDIO_GetFSCfgDesc,                /* GetFSConfigDescriptor */
  USBD_AUDIO_GetOtherSpeedCfgDesc,        /* GetOtherSpeedConfigDescriptor */
  USBD_AUDIO_GetDeviceQualifierDesc       /* GetDeviceQualifierDescriptor */
};

uint8_t USBD_AUDIO_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  (void)cfgidx;

  haudio = (USBD_AUDIO_HandleTypeDef *)USBD_malloc(sizeof(USBD_AUDIO_HandleTypeDef));
  if (haudio == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  (void)USBD_memset(haudio, 0, sizeof(USBD_AUDIO_HandleTypeDef));
  pdev->pClassDataCmsit[pdev->classId] = (void *)haudio;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

  fill_ptr = 0U;
  drain_ptr = 0U;
  capture_tx_busy = 0U;
  haudio->alt_setting = 0U;
  if (((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->AudioInit(
        AUDIO_IN_EP, 48000U, 0U) != 0)
  {
    (void)USBD_free(haudio);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
    return (uint8_t)USBD_FAIL;
  }
  return (uint8_t)USBD_OK;
}

uint8_t USBD_AUDIO_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_AUDIO_HandleTypeDef *haudio;
  (void)cfgidx;

  haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  if (haudio != NULL)
  {
    if (haudio->alt_setting == 1U)
    {
      (void)USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
      pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 0U;
    }
    capture_tx_busy = 0U;
    ((USBD_AUDIO_ItfTypeDef *)pdev->pUserData[pdev->classId])->AudioDeInit(AUDIO_IN_EP);
    (void)USBD_free(haudio);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}
uint8_t USBD_AUDIO_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  static uint8_t status[2] = {0, 0};
  uint8_t interface = (uint8_t)req->wIndex;
  uint8_t alt = (uint8_t)req->wValue;
#ifdef USE_USBD_COMPOSITE
  const uint8_t ac_if = pdev->tclasslist[pdev->classId].Ifs[0];
  const uint8_t as_if = pdev->tclasslist[pdev->classId].Ifs[1];
#else
  const uint8_t ac_if = 0U;
  const uint8_t as_if = 1U;
#endif

  if (haudio == NULL || (interface != ac_if && interface != as_if))
    goto stall;

  if ((req->bmRequest & USB_REQ_TYPE_MASK) != USB_REQ_TYPE_STANDARD)
    goto stall; /* No feature unit or programmable sampling frequency. */

  switch (req->bRequest)
  {
    case USB_REQ_GET_STATUS:
      if (pdev->dev_state == USBD_STATE_CONFIGURED && req->wLength == 2U)
        return (uint8_t)USBD_CtlSendData(pdev, status, 2U);
      break;
    case USB_REQ_GET_DESCRIPTOR:
      if (interface == ac_if && HIBYTE(req->wValue) == 0x24U)
      {
        static uint8_t ac_desc[30];
        /* The AC header's baInterfaceNr is the streaming interface number,
         * which changes from 1 to 3 in the CDC+audio configuration. */
        (void)USBD_memcpy(ac_desc, &AUDIO_FSCfgDesc[18], sizeof(ac_desc));
        ac_desc[8] = as_if;
        return (uint8_t)USBD_CtlSendData(pdev, ac_desc,
                                        MIN(req->wLength, sizeof(ac_desc)));
      }
      break;
    case USB_REQ_GET_INTERFACE:
      if (pdev->dev_state == USBD_STATE_CONFIGURED && req->wLength == 1U)
      {
        static uint8_t current_alt;
        current_alt = interface == as_if ? haudio->alt_setting : 0U;
        return (uint8_t)USBD_CtlSendData(pdev, &current_alt, 1U);
      }
      break;
    case USB_REQ_SET_INTERFACE:
      if (pdev->dev_state != USBD_STATE_CONFIGURED ||
          (interface == ac_if && alt != 0U) || (interface == as_if && alt > 1U))
        break;
      if (interface == as_if && alt != haudio->alt_setting)
      {
        if (alt == 1U)
        {
          if (USBD_LL_OpenEP(pdev, AUDIO_IN_EP, USBD_EP_TYPE_ISOC,
                             AUDIO_IN_PACKET) != USBD_OK)
            break;
          pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 1U;
          drain_ptr = fill_ptr;
        }
        else
        {
          (void)USBD_LL_CloseEP(pdev, AUDIO_IN_EP);
          pdev->ep_in[AUDIO_IN_EP & 0xFU].is_used = 0U;
          capture_tx_busy = 0U;
        }
        haudio->alt_setting = alt;
      }
      return (uint8_t)USBD_OK;
    default:
      break;
  }

stall:
  USBD_CtlError(pdev, req);
  return (uint8_t)USBD_FAIL;
}

uint8_t USBD_AUDIO_EP0_TxSent(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return (uint8_t)USBD_OK;
}

uint8_t USBD_AUDIO_EP0_RxReady(USBD_HandleTypeDef *pdev)
{
  (void)pdev;
  return (uint8_t)USBD_OK;
}
static void USBD_AUDIO_QueuePacket(USBD_HandleTypeDef *pdev);

uint8_t USBD_AUDIO_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  if (epnum == (AUDIO_IN_EP & 0xFU))
  {
    ++audio_in_complete_count;
    capture_tx_busy = 0U;
    /* Queue the following frame immediately, before the next USB SOF. */
    USBD_AUDIO_QueuePacket(pdev);
  }
  return (uint8_t)USBD_OK;
}

uint8_t USBD_AUDIO_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)pdev;
  (void)epnum;
  return (uint8_t)USBD_OK;
}

static void USBD_AUDIO_QueuePacket(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_HandleTypeDef *haudio = (USBD_AUDIO_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t available, count, src;

  if (haudio == NULL || haudio->alt_setting != 1U || capture_tx_busy != 0U)
    return;

  /* SOF primes the first transfer; DataIn keeps subsequent frames queued. */
  (void)USBD_memset(capture_packet, 0, sizeof(capture_packet));
  available = ring_available();
  if (available < AUDIO_IN_PACKET)
    ++audio_iso_underflows;
  count = MIN(available, AUDIO_IN_PACKET);
  src = drain_ptr;
  for (uint32_t i = 0U; i < count; ++i)
  {
    capture_packet[i] = capture_buf[src++];
    if (src == CAPTURE_BUF_SIZE)
      src = 0U;
  }
  drain_ptr = src;
  capture_tx_busy = 1U;
  ++audio_sof_armed_count;
  if (USBD_LL_Transmit(pdev, AUDIO_IN_EP, capture_packet,
                       AUDIO_IN_PACKET) != USBD_OK)
    capture_tx_busy = 0U;
}

uint8_t USBD_AUDIO_SOF(USBD_HandleTypeDef *pdev)
{
  USBD_AUDIO_QueuePacket(pdev);
  return (uint8_t)USBD_OK;
}

uint8_t USBD_AUDIO_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)pdev;
  /* HAL aborted the missed ISO frame and flushed EP3's FIFO.  A fresh SOF
     must be allowed to submit the next frame, otherwise capture stalls. */
  if (epnum == (AUDIO_IN_EP & 0xFU))
  {
    ++audio_in_incomplete_count;
    capture_tx_busy = 0U;
  }
  return (uint8_t)USBD_OK;
}

uint8_t USBD_AUDIO_IsoOUTIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  (void)pdev;
  (void)epnum;
  return (uint8_t)USBD_OK;
}

uint8_t *USBD_AUDIO_GetHSCfgDesc(uint16_t *length)
{
#ifdef USE_USBD_COMPOSITE
  (void)length;
  return NULL;
#else
  *length = (uint16_t)USBD_AUDIO_CONFIG_DESC_SIZ;
  return AUDIO_FSCfgDesc;
#endif
}

uint8_t *USBD_AUDIO_GetFSCfgDesc(uint16_t *length)
{
#ifdef USE_USBD_COMPOSITE
  (void)length;
  return NULL;
#else
  *length = (uint16_t)USBD_AUDIO_CONFIG_DESC_SIZ;
  return AUDIO_FSCfgDesc;
#endif
}

uint8_t *USBD_AUDIO_GetOtherSpeedCfgDesc(uint16_t *length)
{
  (void)length;
  return NULL;
}

uint8_t *USBD_AUDIO_GetDeviceQualifierDesc(uint16_t *length)
{
  (void)length;
  return NULL;
}

uint8_t USBD_AUDIO_RegisterInterface(USBD_HandleTypeDef *pdev,
                                     USBD_AUDIO_ItfTypeDef *fop)
{
  if (fop == NULL)
    return (uint8_t)USBD_FAIL;

  pdev->pUserData[pdev->classId] = fop;
  return (uint8_t)USBD_OK;
}

uint32_t USBD_AUDIO_GetEpPcktSze(USBD_HandleTypeDef *pdev, uint8_t If, uint8_t Ep)
{
  (void)pdev;
  (void)If;
  (void)Ep;
  return (uint32_t)AUDIO_IN_PACKET;
}

void USBD_AUDIO_PushSample(int32_t sample)
{
  uint8_t lo = (uint8_t)(sample & 0xFFU);
  uint8_t hi = (uint8_t)((sample >> 8) & 0xFFU);
  uint8_t top = (uint8_t)((sample >> 16) & 0xFFU);
  uint32_t w = fill_ptr;
  ++audio_samples_produced;

  /* One sample must remain free to distinguish full from empty. */
  uint32_t avail = ring_available();
  if (avail >= CAPTURE_BUF_SIZE - 3U)
  {
    ++audio_samples_dropped;
    return;
  }

  capture_buf[w]     = lo;
  capture_buf[w + 1] = hi;
  capture_buf[w + 2] = top;
  w += 3U;
  if (w >= CAPTURE_BUF_SIZE)
    w = 0U;
  fill_ptr = w;
}
