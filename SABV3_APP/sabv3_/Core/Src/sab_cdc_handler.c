/*
 * sab_cdc_handler.c
 *
 *  SABQ protocol handler for the USB CDC channel (PC GUI <-> DSP).
 *
 *  Receive: raw CDC bytes are enqueued by sab_cdc_feed() from the USB RX
 *  callback into a lock-free SPSC ring (same pattern as the SABU updater
 *  ring in usbd_cdc_if.c).  sab_cdc_process() runs in the main loop and
 *  scans the ring for SABQ frames.
 *
 *  A frame is dispatched by routing it through
 *  sab_intercom_process_rx_buffer() with the matching register address -
 *  the exact path the TouchGFX display used over I2C - so the DSP reacts
 *  identically (change flags, effect rebuilds, save command, FW update).
 *
 *  Responses are built in a static buffer and sent with
 *  CDC_Transmit_HS(), the same transmit path the SABU updater uses.
 */

#include "sab_cdc_handler.h"
#include "sab_cdc_protocol.h"
#include "sab_intercom.h"
#include "SAB_fx_manager.h"
#include "usbd_cdc_if.h"
#include "sdram_memory_handler.h"
#include <string.h>

/* The intercom instance, defined in main.c (noncacheable section). */
extern sab_intercom_tst intercom_st;

/* The fx manager instance, defined in main.c.  Preset/mode commands call
 * the same functions the FSW buttons use (see fsw_process in main.c). */
extern SAB_fx_manager_tst SAB_fx_manager_st;

/* Transmit path shared with the SABU updater (usbd_cdc_if.c).
 * Returns USBD_OK (0) on success, USBD_BUSY while a previous transfer
 * is still in progress. */
extern uint8_t CDC_Transmit_HS(uint8_t *Buf, uint16_t Len);

/* ---------------------------------------------------------------------
 * Receive ring (SPSC, 2 KiB).
 * Writer: CDC_Receive_HS (USB IRQ context) via sab_cdc_feed().
 * Reader: sab_cdc_process() (main loop).
 * ------------------------------------------------------------------- */
#define SABQ_RING_SIZE 2048U

static volatile uint8_t  sabq_ring[SABQ_RING_SIZE];
static volatile uint16_t sabq_head;   /* written by IRQ only */
static volatile uint16_t sabq_tail;   /* read/written by main only */

void sab_cdc_feed(const uint8_t *buf, uint32_t len)
{
    for (uint32_t i = 0; i < len; ++i) {
        uint16_t next = (uint16_t)((sabq_head + 1U) % SABQ_RING_SIZE);
        if (next == sabq_tail) {
            /* Ring full: drop this byte (and the rest of this packet)
             * rather than corrupting the stream.  The parser resyncs
             * on the next magic sequence. */
            return;
        }
        sabq_ring[sabq_head] = buf[i];
        sabq_head = next;
    }
}

/* ---------------------------------------------------------------------
 * Little-endian helpers + CRC-32 (identical to Python zlib.crc32, as
 * used by the SABU updater in usbd_cdc_if.c).
 * ------------------------------------------------------------------- */
static uint16_t q_le16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static uint32_t q_le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static void q_put_u16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
}

static void q_put_u32(uint8_t *p, uint32_t v)
{
    p[0] = (uint8_t)v;
    p[1] = (uint8_t)(v >> 8);
    p[2] = (uint8_t)(v >> 16);
    p[3] = (uint8_t)(v >> 24);
}

static uint32_t q_crc32(const uint8_t *data, uint32_t length)
{
    uint32_t crc = 0xFFFFFFFFUL;
    while (length-- != 0U) {
        crc ^= *data++;
        for (uint8_t bit = 0U; bit < 8U; ++bit) {
            crc = (crc & 1U) ? ((crc >> 1) ^ 0xEDB88320UL) : (crc >> 1);
        }
    }
    return ~crc;
}

/* ---------------------------------------------------------------------
 * Frame parser state machine.
 * ------------------------------------------------------------------- */
typedef enum {
    Q_STATE_SYNC    = 0,   /* scanning for the 4-byte magic */
    Q_STATE_HEADER  = 1,   /* magic found, collecting header bytes */
    Q_STATE_PAYLOAD = 2    /* collecting payload + CRC */
} sabq_state_t;

static sabq_state_t q_state      = Q_STATE_SYNC;
static uint8_t      q_magic_pos;               /* 0..3 bytes of magic matched */
static uint8_t      q_frame[SABQ_MAX_FRAME];   /* assembled frame */
static uint16_t     q_frame_pos;               /* bytes valid in q_frame */
static uint16_t     q_payload_len;            /* from the header */

static void q_reset(void)
{
    q_state     = Q_STATE_SYNC;
    q_magic_pos = 0U;
    q_frame_pos = 0U;
}

/* Forward reference (defined below). */
static void q_dispatch_frame(const uint8_t *frame);

static void q_consume_byte(uint8_t b)
{
    switch (q_state) {
    case Q_STATE_SYNC:
        if (b == 'S') {
            q_magic_pos = 1U;
            q_state     = Q_STATE_HEADER;
            q_frame[0]  = b;
            q_frame_pos = 1U;
        }
        break;

    case Q_STATE_HEADER:
        /* Expect A B Q then the 6 remaining header bytes. */
        q_frame[q_frame_pos++] = b;
        if (q_frame_pos >= SABQ_HDR_SIZE) {
            if ((q_frame[1] == 'A') && (q_frame[2] == 'B') && (q_frame[3] == 'Q')) {
                q_payload_len = q_le16(&q_frame[8]);
                if (q_payload_len > SABQ_MAX_PAYLOAD) {
                    q_reset();                 /* bogus length: resync */
                } else {
                    q_state = Q_STATE_PAYLOAD;
                }
            } else {
                /* Magic mismatch: resync (the new byte may start a
                 * fresh magic - handle in SYNC on the next byte; if
                 * THIS byte is 'S' we want to restart the match now). */
                q_magic_pos = 0U;
                q_state     = Q_STATE_SYNC;
                q_frame_pos = 0U;
                if (b == 'S') {
                    q_state     = Q_STATE_HEADER;
                    q_frame[0]  = b;
                    q_frame_pos = 1U;
                }
            }
        }
        break;

    case Q_STATE_PAYLOAD:
        q_frame[q_frame_pos++] = b;
        if (q_frame_pos >= (uint16_t)(SABQ_HDR_SIZE + q_payload_len + SABQ_CRC_SIZE)) {
            uint32_t expect = q_le32(&q_frame[SABQ_HDR_SIZE + q_payload_len]);
            uint32_t calc   = q_crc32(q_frame, SABQ_HDR_SIZE + q_payload_len);
            q_reset();
            if (calc == expect) {
                q_dispatch_frame(q_frame);
            }
            /* CRC mismatch: frame silently dropped (host times out). */
        }
        break;

    default:
        q_reset();
        break;
    }
}

void sab_cdc_process(void)
{
    for (;;) {
        if (sabq_tail == sabq_head) {
            return;
        }
        uint8_t b = sabq_ring[sabq_tail];
        sabq_tail = (uint16_t)((sabq_tail + 1U) % SABQ_RING_SIZE);
        q_consume_byte(b);
    }
}

/* ---------------------------------------------------------------------
 * Response builder / sender.
 * ------------------------------------------------------------------- */
static uint8_t sabq_response[SABQ_MAX_FRAME] __attribute__((aligned(32)));

static void q_send_response(uint8_t cmd, uint8_t status, uint16_t seq,
                            const uint8_t *data, uint16_t data_len)
{
    /* Status byte is prepended to the payload when the payload is non-empty
     * (mirrors the PC-side parse_frame contract). */
    uint16_t plen = (uint16_t)(data_len + ((data_len != 0U) ? 1U : 0U));

    sabq_response[0] = 'S';
    sabq_response[1] = 'A';
    sabq_response[2] = 'B';
    sabq_response[3] = 'Q';
    sabq_response[4] = SABQ_RESP(cmd);
    sabq_response[5] = SABQ_VERSION;
    q_put_u16(&sabq_response[6], seq);
    q_put_u16(&sabq_response[8], plen);
    if (plen != 0U) {
        sabq_response[SABQ_HDR_SIZE] = status;
        if (data_len != 0U) {
            memcpy(&sabq_response[SABQ_HDR_SIZE + 1U], data, data_len);
        }
    }
    q_put_u32(&sabq_response[SABQ_HDR_SIZE + plen],
              q_crc32(sabq_response, SABQ_HDR_SIZE + plen));

    uint32_t total = (uint32_t)SABQ_HDR_SIZE + plen + SABQ_CRC_SIZE;

    /* CDC_Transmit_HS returns BUSY while the previous multi-packet
     * transfer is still running; retry a bounded number of times. */
    uint8_t tries = 0U;
    do {
        if (CDC_Transmit_HS(sabq_response, (uint16_t)total) == USBD_OK) {
            break;
        }
    } while (++tries < 200U);
}

/* ---------------------------------------------------------------------
 * Dispatch: route every command through the same code path the I2C
 * display used (sab_intercom_process_rx_buffer with the matching
 * register address), or read the intercom union directly.
 * ------------------------------------------------------------------- */
#define Q_NULL_DATA ((const uint8_t *)0)

static void q_dispatch_frame(const uint8_t *frame)
{
    uint8_t  cmd     = frame[4];
    uint16_t seq     = q_le16(&frame[6]);
    uint16_t plen    = q_le16(&frame[8]);
    uint8_t *payload = (uint8_t *)&frame[SABQ_HDR_SIZE];

    switch (cmd) {

    /* ---- handshake ------------------------------------------------ */
    case SABQ_CMD_HELLO: {
        uint8_t data[6];
        q_put_u32(&data[0], SABQ_PROTOCOL_ID);
        data[4] = intercom_st.info_un.dsp_info_st.dsp_fw_ver_major_u8;
        data[5] = intercom_st.info_un.dsp_info_st.dsp_fw_ver_minor_u8;
        q_send_response(cmd, SABQ_STATUS_OK, seq, data, sizeof(data));
        break;
    }

    /* ---- CPU load: FX cycle cost per sample (DWT CYCCNT) ---------------- */
    case SABQ_CMD_GET_CPULOAD: {
        uint8_t  cbuf[16U];
        uint32_t avg, peak, last;
        SAB_fx_get_cpu_cycles(&avg, &peak, &last);
        q_put_u32(&cbuf[ 0U], avg);
        q_put_u32(&cbuf[ 4U], peak);
        q_put_u32(&cbuf[ 8U], last);
        q_put_u32(&cbuf[12U], SABQ_CPU_BLOCK_BUDGET);
        q_send_response(cmd, SABQ_STATUS_OK, seq, cbuf, sizeof(cbuf));
        break;
    }

    case SABQ_CMD_GET_PRESET_SAVEFILE: {
        if (plen != 4U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint32_t offset = q_le32(payload);
        uint32_t total_size = SAB_preset_savefile_size();
        if (offset >= total_size) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint16_t chunk_size = (uint16_t)(total_size - offset);
        if (chunk_size > SABQ_PRESET_SAVEFILE_CHUNK_SIZE) {
            chunk_size = SABQ_PRESET_SAVEFILE_CHUNK_SIZE;
        }
        uint8_t data[4U + SABQ_PRESET_SAVEFILE_CHUNK_SIZE];
        q_put_u32(data, total_size);
        if (!SAB_read_preset_savefile(offset, &data[4], chunk_size)) {
            q_send_response(cmd, SABQ_STATUS_INTERNAL, seq, Q_NULL_DATA, 0U);
            break;
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, data,
                        (uint16_t)(4U + chunk_size));
        break;
    }

    /* ---- full-store preset restore (RAM-staged, CRC-gated commit) ----- */
    case SABQ_CMD_PRESET_RESTORE_BEGIN: {
        if (plen != 8U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint32_t size = q_le32(&payload[0]);
        uint32_t crc  = q_le32(&payload[4]);
        if (!SAB_preset_restore_begin(size, crc)) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_PRESET_RESTORE_CHUNK: {
        if (plen < 5U || plen > 4U + SABQ_PRESET_SAVEFILE_CHUNK_SIZE) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint32_t offset = q_le32(payload);
        if (!SAB_preset_restore_chunk(offset, &payload[4],
                                      (uint16_t)(plen - 4U))) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_PRESET_RESTORE_COMMIT: {
        if (plen != 0U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        /* BAD_ARG: either an invalid image (incomplete / CRC mismatch ->
         * existing flash kept) or the flash write itself failed. */
        if (SAB_preset_restore_commit(&SAB_fx_manager_st)) {
            q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        } else {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
        }
        break;
    }

    case SABQ_CMD_PRESET_RESTORE_ABORT:
        SAB_preset_restore_abort();
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;

    case SABQ_CMD_GET_INFO:
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        (const uint8_t *)&intercom_st.info_un.all_u8,
                        (uint16_t)sizeof(intercom_st.info_un.all_u8));
        break;

    case SABQ_CMD_GET_PRESET:
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        (const uint8_t *)&intercom_st.preset_data_un.all_u32,
                        SAB_I2C_REG_PRESETNUM_LEN);
        break;

    case SABQ_CMD_GET_LOOP: {
        uint8_t n = (plen >= 1U) ? payload[0] : 0U;
        if (n < 1U || n > NUM_OF_LOOPS) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        (const uint8_t *)&intercom_st.loop_data[n - 1U],
                        (uint16_t)sizeof(sab_loop_num_tun));
        break;
    }

    case SABQ_CMD_GET_PARAM: {
        uint8_t fx_idx = (plen >= 1U) ? payload[0] : 0U;
        uint8_t count  = (plen >= 2U) ? payload[1] : 0U;

        if (fx_idx >= NUM_OF_MAX_PARAMS) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        if (count == 0U) {
            count = 1U;
        }
        if (count > NUM_OF_MAX_PARAMS) {
            count = NUM_OF_MAX_PARAMS;
        }
        if (intercom_st.fx_param_pun[fx_idx] == NULL) {
            q_send_response(cmd, SABQ_STATUS_INTERNAL, seq, Q_NULL_DATA, 0U);
            break;
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        (const uint8_t *)&intercom_st.fx_param_pun[fx_idx][0],
                        (uint16_t)(count * SAB_I2C_REG_FXPARAM_LEN));
        break;
    }

    /* ---- DIAG: dump fx_param_pun pointer values ---------------------- */
    case SABQ_CMD_GET_FXPTR: {
        uint8_t start = (plen >= 1U) ? payload[0] : 0U;
        uint8_t cnt   = (plen >= 2U) ? payload[1] : 12U;
        if (start >= NUM_OF_MAX_PARAMS) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        if (cnt == 0U) {
            cnt = 1U;
        }
        if ((uint16_t)start + (uint16_t)cnt > (uint16_t)NUM_OF_MAX_PARAMS) {
            cnt = (uint8_t)(NUM_OF_MAX_PARAMS - start);
        }
        uint8_t pbuf[12U * 4U];
        for (uint8_t i = 0U; i < cnt; ++i) {
            q_put_u32(&pbuf[(uint16_t)i * 4U],
                      (uint32_t)intercom_st.fx_param_pun[start + i]);
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, pbuf, (uint16_t)(cnt * 4U));
        break;
    }

    case SABQ_CMD_GET_BYPASS:
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        (const uint8_t *)&intercom_st.loopbypass_un.all_u8,
                        SAB_I2C_REG_LOOPBYPASSSTATE_LEN);
        break;

    case SABQ_CMD_GET_EFFECTS: {
        if (intercom_st.implemented_fx_data_ptr == NULL) {
            q_send_response(cmd, SABQ_STATUS_INTERNAL, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint8_t n = intercom_st.num_of_implemented_effects;
        if (n > 33U) {                       /* fits SABQ_MAX_PAYLOAD */
            n = 33U;
        }
        uint8_t data[1U + 33U * (uint16_t)sizeof(fx_data_tst)];
        data[0] = n;
        for (uint8_t i = 0U; i < n; ++i) {
            memcpy(&data[1U + (uint16_t)i * (uint16_t)sizeof(fx_data_tst)],
                   &intercom_st.implemented_fx_data_ptr[i],
                   sizeof(fx_data_tst));
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, data,
                        (uint16_t)(1U + (uint16_t)n * (uint16_t)sizeof(fx_data_tst)));
        break;
    }

    case SABQ_CMD_GET_CURRENT_FX:
        q_send_response(cmd, SABQ_STATUS_OK, seq,
                        &intercom_st.current_fx_in_edit, 1U);
        break;

    /* ---- mem telemetry: pool (float slots) + C-heap (bytes) -------- */
    case SABQ_CMD_GET_MEMSTATS: {
        uint8_t mbuf[68U];
        uint32_t i;
        q_put_u32(&mbuf[ 0U], sdram_pool_used_floats());
        q_put_u32(&mbuf[ 4U], sdram_pool_total_floats());
        q_put_u32(&mbuf[ 8U], sab_heap_used_bytes());
        q_put_u32(&mbuf[12U], sab_heap_free_bytes());
        q_put_u32(&mbuf[16U], sab_heap_total_bytes());
        /* per-slot effect struct bytes (index = fx_idx) so the PC can show
         * exactly how much memory each allocated effect occupies */
        for (i = 0U; i < 12U; i++) {
            q_put_u32(&mbuf[(uint16_t)(20U + (i * 4U))],
                      SAB_fx_slot_size_bytes(&SAB_fx_manager_st, (int)i));
        }
        q_send_response(cmd, SABQ_STATUS_OK, seq, mbuf, sizeof(mbuf));
        break;
    }

    /* ---- writes (I2C register semantics) --------------------------- */
    case SABQ_CMD_SET_PARAM: {
        /* Same as the display writing FXPARAM<slot> over I2C: the full
         * 7-byte parameter union lands in the slot's parameter, which
         * sets change_occured_flg -> effect rebuild in the main loop. */
        if (plen < 3U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint8_t fx_idx = payload[0];
        uint8_t slot   = payload[1];   /* 1-based parameter slot */
        if ((fx_idx >= NUM_OF_MAX_PARAMS) || (slot < 1U) || (slot > NUM_OF_MAX_PARAMS)) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        if (intercom_st.fx_param_pun[fx_idx] == NULL) {
            q_send_response(cmd, SABQ_STATUS_INTERNAL, seq, Q_NULL_DATA, 0U);
            break;
        }
        if (intercom_st.current_fx_in_edit != fx_idx) {
            intercom_st.current_fx_in_edit = fx_idx;
        }
        intercom_st.fx_param_pun[fx_idx][slot - 1U].value_u8 = payload[2];
        intercom_st.register_addr_u8 = (uint8_t)(SAB_I2C_REG_FXPARAM1 + slot - 1U);
        intercom_st.process_rx_buffer(&intercom_st,
                                      (uint8_t *)&intercom_st.fx_param_pun[fx_idx][slot - 1U],
                                      SAB_I2C_REG_FXPARAM_LEN);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_SET_LOOP: {
        if (plen < (uint16_t)(1U + sizeof(sab_loop_num_tun))) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        uint8_t n = payload[0];
        if (n < 1U || n > NUM_OF_LOOPS) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        intercom_st.register_addr_u8 = (uint8_t)(SAB_I2C_REG_LOOP1FX + n - 1U);
        intercom_st.process_rx_buffer(&intercom_st, &payload[1U],
                                      (uint32_t)sizeof(sab_loop_num_tun));
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_SET_BYPASS: {
        if (plen < SAB_I2C_REG_LOOPBYPASSSTATE_LEN) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        intercom_st.register_addr_u8 = SAB_I2C_REG_LOOPBYPASSSTATE;
        intercom_st.process_rx_buffer(&intercom_st, payload,
                                      SAB_I2C_REG_LOOPBYPASSSTATE_LEN);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_SAVE_PRESET: {
        uint32_t flash_error = SAB_save_preset_to_flash(&SAB_fx_manager_st);
        if(flash_error != 0U){
            uint8_t error_data[4];
            q_put_u32(error_data, flash_error);
            q_send_response(cmd, SABQ_STATUS_INTERNAL, seq, error_data,
                            sizeof(error_data));
        }else{
            q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        }
        break;
    }

    case SABQ_CMD_FW_UPDATE_REQ: {
        uint8_t up_req = 1U;
        intercom_st.register_addr_u8 = SAB_I2C_REG_DSP_FW_UPDATE;
        intercom_st.process_rx_buffer(&intercom_st, &up_req, 1U);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        /* main loop sees dsp_fw_update_flg and jumps to the bootloader. */
        break;
    }

    case SABQ_CMD_SET_CURRENT_FX: {
        if (plen < 1U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        if (payload[0] >= NUM_OF_MAX_PARAMS) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        intercom_st.register_addr_u8 = SAB_I2c_REG_CURRENT_FX;
        intercom_st.process_rx_buffer(&intercom_st, &payload[0], 1U);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    case SABQ_CMD_PRESET_UP:
        /* Same call the FS1+FS2 chord makes in fsw_process(). */
        SAB_preset_up_pressed(&SAB_fx_manager_st);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;

    case SABQ_CMD_PRESET_DOWN:
        /* Same call the FS3+FS4 chord makes in fsw_process(). */
        SAB_preset_down_pressed(&SAB_fx_manager_st);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;

    case SABQ_CMD_SET_MODE: {
        /* req: [mode 0..3 = PRESET_MODE_A..D]; out-of-range clamps to A
         * inside SAB_set_preset_mode(). */
        if (plen < 1U) {
            q_send_response(cmd, SABQ_STATUS_BAD_ARG, seq, Q_NULL_DATA, 0U);
            break;
        }
        SAB_set_preset_mode(&SAB_fx_manager_st, (fx_active_modes_ten)payload[0]);
        q_send_response(cmd, SABQ_STATUS_OK, seq, Q_NULL_DATA, 0U);
        break;
    }

    default:
        q_send_response(cmd, SABQ_STATUS_BAD_CMD, seq, Q_NULL_DATA, 0U);
        break;
    }
}
