/*
 * Copyright (c) 2024
 * SPDX-License-Identifier: GPL 
 *
 * SH1107 OLED display controller register definitions.
 * Datasheet: https://www.displayfuture.com/Display/datasheet/controller/SH1107.pdf
 *
 * The SH1107 is a 128x128 monochrome OLED/PLED driver IC.
 * It supports page addressing mode only (no horizontal/vertical addressing
 * like SSD1306). Each page is 8 pixels high, so 128 vertical pixels = 16 pages.
 * For a 64x128 display, only 8 pages are used.
 */

#ifndef SH1107_REGS_H
#define SH1107_REGS_H

/* ── Fundamental Commands ──────────────────────────────────────────────── */

/* Set lower column address: 0x00 – 0x0F (lower nibble) */
#define SH1107_SET_LOWER_COL_ADDR           0x00

/* Set upper column address: 0x10 – 0x1F (upper nibble) */
#define SH1107_SET_UPPER_COL_ADDR           0x10

/* Set memory addressing mode (page mode is the only mode on SH1107)
 * 0x20 = page addressing mode (default)
 * 0x21 = vertical addressing mode (non-standard, not on all variants)
 */
#define SH1107_SET_MEM_ADDR_MODE            0x20

/* Set contrast control: 2-byte command, second byte 0x00–0xFF */
#define SH1107_SET_CONTRAST                 0x81

/* Segment re-map:
 * 0xA0 = column address 0 mapped to SEG0 (normal)
 * 0xA1 = column address 127 mapped to SEG0 (flipped)
 */
#define SH1107_SET_SEGMENT_REMAP_NORMAL     0xA0
#define SH1107_SET_SEGMENT_REMAP_REVERSE    0xA1

/* Set multiplex ratio: 2-byte command, second byte = MUX-1 (0x00–0x7F) */
#define SH1107_SET_MULTIPLEX_RATIO          0xA8

/* Entire display ON:
 * 0xA4 = output follows RAM content
 * 0xA5 = entire display ON (all pixels lit, ignores RAM)
 */
#define SH1107_DISPLAY_RAM                  0xA4
#define SH1107_DISPLAY_ALL_ON               0xA5

/* Set normal/inverse display:
 * 0xA6 = normal (1 in RAM = pixel ON)
 * 0xA7 = inverse (0 in RAM = pixel ON)
 */
#define SH1107_DISPLAY_NORMAL               0xA6
#define SH1107_DISPLAY_INVERSE              0xA7

/* Set display offset: 2-byte command (0xD3), second byte 0x00–0x7F
 * This shifts the display vertically by the specified number of COM lines.
 * For 64x128 displays, typically set to 0x60 (96).
 */
#define SH1107_SET_DISPLAY_OFFSET           0xD3

/* Set display clock divide ratio / oscillator frequency:
 * 2-byte command. Bits [3:0] = divide ratio - 1, bits [7:4] = osc freq
 */
#define SH1107_SET_CLOCK_DIV_RATIO          0xD5

/* Set pre-charge period: 2-byte command.
 * Bits [3:0] = phase 1 period, bits [7:4] = phase 2 period
 */
#define SH1107_SET_PRECHARGE_PERIOD         0xD9

/* Set VCOM deselect level: 2-byte command, second byte sets level */
#define SH1107_SET_VCOM_DESELECT            0xDB

/* Set display start line: 2-byte command (0xDC), second byte 0x00–0x7F */
#define SH1107_SET_DISPLAY_START_LINE       0xDC

/* Set page address: 0xB0–0xBF for page 0–15 */
#define SH1107_SET_PAGE_ADDR                0xB0

/* Set COM output scan direction:
 * 0xC0 = normal (COM0 to COM[N-1])
 * 0xC8 = remapped (COM[N-1] to COM0)
 */
#define SH1107_SET_COM_SCAN_NORMAL          0xC0
#define SH1107_SET_COM_SCAN_REVERSE         0xC8

/* Set COM pins hardware configuration: 2-byte command (0xDA)
 * Second byte configures the COM pins output.
 * 0x12 is typical for 64x128 displays.
 */
#define SH1107_SET_COM_PINS                 0xDA

/* ── Charge Pump Commands ──────────────────────────────────────────────── */

/* Set DC-DC converter: 2-byte command
 * 0xAD followed by:
 *   0x8A = DC-DC OFF
 *   0x8B = DC-DC ON (internal pump)
 */
#define SH1107_SET_DCDC                     0xAD
#define SH1107_DCDC_OFF                     0x8A
#define SH1107_DCDC_ON                      0x8B

/* ── Display ON/OFF ────────────────────────────────────────────────────── */

#define SH1107_DISPLAY_OFF                  0xAE
#define SH1107_DISPLAY_ON                   0xAF

/* ── I2C Protocol Control Bytes ────────────────────────────────────────── */

/* Control byte: Co=1, D/C=0 → command byte follows, more commands may follow */
#define SH1107_I2C_CONTROL_CMD_CONT         0x80

/* Control byte: Co=0, D/C=0 → last command byte (or stream of commands) */
#define SH1107_I2C_CONTROL_CMD_LAST         0x00

/* Control byte: Co=0, D/C=1 → data bytes follow */
#define SH1107_I2C_CONTROL_DATA             0x40

/* ── Display Dimensions ────────────────────────────────────────────────── */

#define SH1107_COLUMNS                      128
#define SH1107_PAGES_MAX                    16   /* 128 / 8 */
#define SH1107_PIXELS_PER_PAGE              8

#endif /* SH1107_REGS_H */
