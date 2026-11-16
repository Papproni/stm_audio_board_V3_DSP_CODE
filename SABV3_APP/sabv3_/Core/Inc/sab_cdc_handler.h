/*
 * sab_cdc_handler.h
 *
 *  SABQ protocol handler: parses PC-GUI request frames from the USB CDC
 *  byte stream, applies them to the intercom with the same register
 *  semantics the I2C display used, and sends response frames back.
 *
 *  Runs alongside the SABU firmware-updater protocol on the same CDC
 *  port: both protocols scan the same byte stream for their own magic.
 */

#ifndef SAB_CDC_HANDLER_H
#define SAB_CDC_HANDLER_H

#include <stdint.h>

/* Feed raw CDC bytes into the handler.
 * Called from the USB receive callback (CDC_Receive_HS) - i.e. USB IRQ
 * context.  Only enqueues into a ring buffer; no dispatch here.
 * SPSC: single writer (CDC RX), single reader (sab_cdc_process). */
void sab_cdc_feed(const uint8_t *buf, uint32_t len);

/* Parse SABQ frames from the ring, dispatch them, and send responses.
 * Call from the main loop next to FW_Update_Process(). */
void sab_cdc_process(void);

#endif /* SAB_CDC_HANDLER_H */
