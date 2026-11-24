/*
 * sab_cdc_protocol.h
 *
 *  SABQ - PC-GUI protocol over the USB CDC channel.
 *
 *  This is the wire format used by the PC application that replaces the
 *  TouchGFX display unit.  It carries the SAME data as the original I2C
 *  intercom protocol (same register semantics, same data layouts): the
 *  handler (sab_cdc_handler.c) feeds frames through
 *  sab_intercom_process_rx_buffer() so the DSP sees exactly what it saw
 *  when the display wrote a register over I2C.
 *
 *  Frame layout (little-endian on the wire):
 *
 *    offset  size  field
 *    0       4     magic "SABQ" (0x53 0x41 0x42 0x51)
 *    4       1     command
 *    5       1     protocol version (0x01)
 *    6       2     sequence number (echoed in the response)
 *    8       2     payload length (0..512)
 *    10      len   payload
 *    10+len  4     CRC-32 (zlib) over bytes 0 .. 10+len
 *
 *  Stop-and-wait: the host waits for the response frame before sending
 *  the next request.  A response has the same layout; its command byte
 *  is the request command | 0x80 and the first payload byte (when the
 *  payload is non-empty) is a status code.
 */

#ifndef SAB_CDC_PROTOCOL_H
#define SAB_CDC_PROTOCOL_H

#include <stdint.h>

/* "SABQ" read back as a little-endian u32.  Doubles as the protocol id
 * reported by HELLO (identical to the PC-side PROTOCOL_ID). */
#define SABQ_PROTOCOL_ID      0x51424153UL

#define SABQ_VERSION          0x01U

#define SABQ_HDR_SIZE         10U
#define SABQ_CRC_SIZE         4U
#define SABQ_MAX_PAYLOAD      512U
#define SABQ_MAX_FRAME        (SABQ_HDR_SIZE + SABQ_MAX_PAYLOAD + SABQ_CRC_SIZE)

/* Commands (host -> device) --------------------------------------------*/
enum {
    /* Handshake */
    SABQ_CMD_HELLO            = 0x01,

    /* Reads (device state -> host) */
    SABQ_CMD_GET_INFO         = 0x10,  /* info register (12 bytes)         */
    SABQ_CMD_GET_PRESET       = 0x11,  /* preset number (4 bytes)          */
    SABQ_CMD_GET_LOOP         = 0x12,  /* req: [loop 1..4] -> 42 bytes     */
    SABQ_CMD_GET_PARAM        = 0x13,  /* req: [fx_idx, count] -> count*7  */
    SABQ_CMD_GET_BYPASS       = 0x14,  /* bypass states (3 bytes)          */
    SABQ_CMD_GET_EFFECTS      = 0x15,  /* implemented effect table         */
    SABQ_CMD_GET_CURRENT_FX   = 0x16,  /* fx being edited (1 byte)         */
    SABQ_CMD_GET_FXPTR        = 0x17,  /* DIAG: req [start,count] -> ptrs  */
    SABQ_CMD_GET_MEMSTATS     = 0x18,  /* mem telemetry: 5 LE u32 (pool+)  */
    SABQ_CMD_GET_CPULOAD      = 0x19,  /* cpu telemetry: 4 LE u32 cyc/block*/
    SABQ_CMD_GET_PRESET_SAVEFILE = 0x1A, /* req [offset:u32] -> size + bytes */

    /* Writes (host -> device state, I2C register semantics) */
    SABQ_CMD_SET_PARAM        = 0x20,  /* req: [fx_idx, slot 1..12, value] */
    SABQ_CMD_SET_LOOP         = 0x21,  /* req: [loop 1..4] + 42 bytes      */
    SABQ_CMD_SET_BYPASS       = 0x22,  /* req: [3 bytes]                   */
    SABQ_CMD_SAVE_PRESET      = 0x23,  /* req: empty -> commit active preset to flash */
    SABQ_CMD_FW_UPDATE_REQ    = 0x24,  /* req: empty -> dsp_fw_update_flg  */
    SABQ_CMD_SET_CURRENT_FX   = 0x25,  /* req: [fx_idx 0..11]              */
    /* Preset / mode control - the same code path the FSW buttons use
     * (SAB_preset_up/down_pressed, SAB_set_preset_mode on the global
     * SAB_fx_manager_st). */
    SABQ_CMD_PRESET_UP        = 0x26,  /* req: empty -> next preset        */
    SABQ_CMD_PRESET_DOWN      = 0x27,  /* req: empty -> previous preset    */
    SABQ_CMD_SET_MODE         = 0x28,  /* req: [mode 0..3 = A..D]          */
    /* Full-store restore: stage a backup image in RAM; the preset sector is
     * erased + reprogrammed only at COMMIT, and only when the whole-image
     * CRC-32 matches. A corrupt/truncated backup never touches flash. */
    SABQ_CMD_PRESET_RESTORE_BEGIN  = 0x29, /* req: [size:u32, crc32:u32]   */
    SABQ_CMD_PRESET_RESTORE_CHUNK  = 0x2A, /* req: [offset:u32, <=480 bytes]*/
    SABQ_CMD_PRESET_RESTORE_COMMIT = 0x2B, /* req: empty -> verify + write */
    SABQ_CMD_PRESET_RESTORE_ABORT  = 0x2C, /* req: empty -> discard staged */
};

/* CPU budget per sample: 1/48000 s @ 550 MHz = 11458 cycles (one SAI sample per
 * DMA completion; the FX pass computes all four inputs for that single sample).
 * GET_CPULOAD reports it so the host can turn measured cycles into a % of CPU. */
#define SABQ_CPU_BLOCK_BUDGET   11458U
#define SABQ_PRESET_SAVEFILE_CHUNK_SIZE 480U

/* Response command = request command | 0x80 */
#define SABQ_RESP(cmd)        ((uint8_t)((cmd) | 0x80U))

/* Response status codes (first byte of a non-empty payload) -------------*/
enum {
    SABQ_STATUS_OK            = 0x00,
    SABQ_STATUS_BAD_CMD       = 0x01,
    SABQ_STATUS_BAD_ARG       = 0x02,
    SABQ_STATUS_INTERNAL      = 0xFF,
};

#endif /* SAB_CDC_PROTOCOL_H */
