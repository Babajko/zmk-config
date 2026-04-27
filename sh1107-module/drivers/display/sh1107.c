/*
 * Copyright (c) 2024
 * SPDX-License-Identifier: GPL 
 *
 * SH1107 OLED display driver for Zephyr (I2C only).
 *
 * Designed for 64x128 and 128x128 monochrome OLED panels driven by
 * the Sino Wealth SH1107 controller IC.
 *
 * Key differences from SSD1306:
 *  - Page addressing only (no horizontal/vertical addressing modes).
 *  - Page address is set per-page, column address auto-increments within page.
 *  - Display start line is a 2-byte command (0xDC + offset).
 *  - DC-DC converter enable is 0xAD + 0x8B (not 0x8D + 0x14).
 *  - No continuous horizontal scroll hardware.
 *
 * This driver implements the Zephyr display_driver_api and works with
 * LVGL, CFB, and direct framebuffer writes through display_write().
 */

#define DT_DRV_COMPAT sinowealth_sh1107

#include <string.h>
#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include "sh1107_regs.h"

LOG_MODULE_REGISTER(sh1107, CONFIG_DISPLAY_LOG_LEVEL);

/* ── Internal data structures ──────────────────────────────────────────── */

struct sh1107_config {
	struct i2c_dt_spec i2c;
	uint16_t width;
	uint16_t height;
	uint8_t  segment_offset;
	uint8_t  page_offset;
	uint8_t  display_offset;
	uint8_t  multiplex_ratio;
	uint8_t  prechargep;
	uint8_t  contrast;
	uint8_t  clock_div_ratio;
	uint8_t  vcom_deselect;
	bool     segment_remap;
	bool     com_invdir;
	bool     inversion_on;
	bool     dcdc_on;
};

struct sh1107_data {
	bool blanking_on;
};

/* ── I2C helpers ───────────────────────────────────────────────────────── */

/**
 * Write a command byte sequence to SH1107 over I2C.
 * Each command byte is prefixed with control byte 0x80 (Co=1, D/C=0),
 * except the last which uses 0x00 (Co=0, D/C=0).
 */
static int sh1107_write_cmd(const struct device *dev, const uint8_t *cmd,
			    size_t cmd_len)
{
	const struct sh1107_config *config = dev->config;
	uint8_t buf[64]; /* commands are always short */
	size_t pos = 0;

	if (cmd_len * 2 > sizeof(buf)) {
		LOG_ERR("Command too long: %zu", cmd_len);
		return -ENOMEM;
	}

	for (size_t i = 0; i < cmd_len; i++) {
		/* Control byte */
		buf[pos++] = (i < cmd_len - 1)
			? SH1107_I2C_CONTROL_CMD_CONT
			: SH1107_I2C_CONTROL_CMD_LAST;
		buf[pos++] = cmd[i];
	}

	return i2c_write_dt(&config->i2c, buf, pos);
}

static inline int sh1107_write_cmd_byte(const struct device *dev, uint8_t cmd)
{
	return sh1107_write_cmd(dev, &cmd, 1);
}

static inline int sh1107_write_cmd_pair(const struct device *dev,
					uint8_t cmd, uint8_t arg)
{
	uint8_t buf[2] = { cmd, arg };
	return sh1107_write_cmd(dev, buf, 2);
}

/**
 * Write display data (GDDRAM) over I2C.
 * Prefix with control byte 0x40 (Co=0, D/C=1) then raw pixel data.
 */
static int sh1107_write_data(const struct device *dev, const uint8_t *data,
			     size_t len)
{
	const struct sh1107_config *config = dev->config;
	/* We send control byte + data.  For large transfers, use
	 * i2c_burst_write which prepends a register byte.
	 */
	return i2c_burst_write_dt(&config->i2c, SH1107_I2C_CONTROL_DATA,
				  data, len);
}

/* ── Set page/column address ───────────────────────────────────────────── */

static int sh1107_set_pos(const struct device *dev, uint8_t page, uint8_t col)
{
	const struct sh1107_config *config = dev->config;
	uint8_t col_with_offset = col + config->segment_offset;
	
	/*
	 * U8g2 uses this order (verified working for 64x128):
	 * 1. Upper column nibble (0x10 | high)  - FIRST!
	 * 2. Lower column nibble (0x00 | low)
	 * 3. Page address (0xB0 | page)         - LAST!
	 */
	uint8_t cmds[] = {
		(uint8_t)(SH1107_SET_UPPER_COL_ADDR | ((col_with_offset >> 4) & 0x0F)),
		(uint8_t)(SH1107_SET_LOWER_COL_ADDR | (col_with_offset & 0x0F)),
		(uint8_t)(SH1107_SET_PAGE_ADDR | (page & 0x0F)),
	};

	return sh1107_write_cmd(dev, cmds, ARRAY_SIZE(cmds));
}

/* ── Display API implementation ────────────────────────────────────────── */

static int sh1107_blanking_on(const struct device *dev)
{
	struct sh1107_data *data = dev->data;
	int ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_OFF);

	if (ret == 0) {
		data->blanking_on = true;
	}
	return ret;
}

static int sh1107_blanking_off(const struct device *dev)
{
	struct sh1107_data *data = dev->data;
	int ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_ON);

	if (ret == 0) {
		data->blanking_on = false;
	}
	return ret;
}

static int sh1107_write(const struct device *dev, const uint16_t x,
			const uint16_t y,
			const struct display_buffer_descriptor *desc,
			const void *buf)
{
	const struct sh1107_config *config = dev->config;
	const uint8_t *src = buf;
	int ret;

	if (buf == NULL || desc->buf_size == 0U) {
		return -EINVAL;
	}

	/*
	 * LVGL sends MONO_VLSB format (vertical bytes, LSB = top):
	 *   - pitch = width (bytes per page row)
	 *   - Buffer layout matches SH1107 RAM format
	 */
	uint16_t pitch = desc->pitch;
	uint8_t start_page = y >> 3;
	uint8_t num_pages = (desc->height + 7) >> 3;
	
	for (uint8_t pg = 0; pg < num_pages; pg++) {
		uint8_t page = start_page + pg + config->page_offset;
		const uint8_t *page_data = src + (pg * pitch);
		
		ret = sh1107_set_pos(dev, page, x);
		if (ret < 0) return ret;
		
		uint16_t write_len = desc->width;
		if (write_len > 64) write_len = 64;
		
		ret = sh1107_write_data(dev, page_data, write_len);
		if (ret < 0) return ret;
	}

	return 0;
}

static int sh1107_read(const struct device *dev, const uint16_t x,
		       const uint16_t y,
		       const struct display_buffer_descriptor *desc,
		       void *buf)
{
	/* SH1107 I2C does not support read-back of GDDRAM */
	return -ENOTSUP;
}

static void *sh1107_get_framebuffer(const struct device *dev)
{
	return NULL;
}

static int sh1107_set_brightness(const struct device *dev,
				 const uint8_t brightness)
{
	return -ENOTSUP;
}

static int sh1107_set_contrast(const struct device *dev,
			       const uint8_t contrast)
{
	return sh1107_write_cmd_pair(dev, SH1107_SET_CONTRAST, contrast);
}

static void sh1107_get_capabilities(const struct device *dev,
				    struct display_capabilities *caps)
{
	const struct sh1107_config *config = dev->config;

	memset(caps, 0, sizeof(*caps));
	caps->x_resolution = config->width;
	caps->y_resolution = config->height;
	caps->supported_pixel_formats = PIXEL_FORMAT_MONO10 |
					PIXEL_FORMAT_MONO01;
	caps->current_pixel_format = PIXEL_FORMAT_MONO10;
	caps->current_orientation = DISPLAY_ORIENTATION_NORMAL;
	/*
	 * SCREEN_INFO_MONO_VTILED: each byte = 8 vertical pixels
	 * No MONO_MSB_FIRST: bit 0 = top row (LSB first)
	 * This tells LVGL to format data for vertical-byte displays.
	 */
	caps->screen_info = SCREEN_INFO_MONO_VTILED;
}

static int sh1107_set_pixel_format(const struct device *dev,
				   const enum display_pixel_format pf)
{
	int ret;

	switch (pf) {
	case PIXEL_FORMAT_MONO10:
		ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_NORMAL);
		break;
	case PIXEL_FORMAT_MONO01:
		ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_INVERSE);
		break;
	default:
		return -ENOTSUP;
	}

	return ret;
}

static int sh1107_set_orientation(const struct device *dev,
				  const enum display_orientation orientation)
{
	return -ENOTSUP;
}

/* ── Display driver API structure ──────────────────────────────────────── */

static const struct display_driver_api sh1107_driver_api = {
	.blanking_on      = sh1107_blanking_on,
	.blanking_off     = sh1107_blanking_off,
	.write            = sh1107_write,
	.read             = sh1107_read,
	.get_framebuffer  = sh1107_get_framebuffer,
	.set_brightness   = sh1107_set_brightness,
	.set_contrast     = sh1107_set_contrast,
	.get_capabilities = sh1107_get_capabilities,
	.set_pixel_format = sh1107_set_pixel_format,
	.set_orientation  = sh1107_set_orientation,
};

/* ── Initialization ────────────────────────────────────────────────────── */

static int sh1107_init(const struct device *dev)
{
	const struct sh1107_config *config = dev->config;
	struct sh1107_data *data = dev->data;
	int ret;

	LOG_DBG("Initializing SH1107 display");

	if (!i2c_is_ready_dt(&config->i2c)) {
		LOG_ERR("I2C bus %s not ready", config->i2c.bus->name);
		return -ENODEV;
	}

	/*
	 * Init sequence based on U8g2 u8x8_d_sh1107_64x128_noname_init_seq
	 * for QG-6428TSWKG01 (Chinese 64x128 OLED):
	 *
	 * Key insight: 0xD3 (display offset) = 0x60 shifts the 64px wide
	 * display to use the center of the 128-segment controller.
	 */

	/* 0xAE - Display OFF during init */
	ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_OFF);
	if (ret < 0) {
		LOG_ERR("Failed to turn display off: %d", ret);
		return ret;
	}

	/* 0xDC, 0x00 - Start line = 0 (always 0 for this display) */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_DISPLAY_START_LINE, 0x00);
	if (ret < 0) { return ret; }

	/* 0x81, contrast - Contrast control */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_CONTRAST, config->contrast);
	if (ret < 0) { return ret; }

	/* 0x20 - Memory addressing mode (page mode) */
	ret = sh1107_write_cmd_byte(dev, SH1107_SET_MEM_ADDR_MODE);
	if (ret < 0) { return ret; }

	/* 0xA0/0xA1 - Segment remap */
	ret = sh1107_write_cmd_byte(dev,
		config->segment_remap ? SH1107_SET_SEGMENT_REMAP_REVERSE
				      : SH1107_SET_SEGMENT_REMAP_NORMAL);
	if (ret < 0) { return ret; }

	/* 0xC0/0xC8 - COM scan direction */
	ret = sh1107_write_cmd_byte(dev,
		config->com_invdir ? SH1107_SET_COM_SCAN_REVERSE
				   : SH1107_SET_COM_SCAN_NORMAL);
	if (ret < 0) { return ret; }

	/* 0xA8, multiplex - Multiplex ratio */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_MULTIPLEX_RATIO,
				    config->multiplex_ratio);
	if (ret < 0) { return ret; }

	/* 0xD3, display_offset - Display offset (CRITICAL for 64x128!) */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_DISPLAY_OFFSET,
				    config->display_offset);
	if (ret < 0) { return ret; }

	/* 0xD5, clock_div - Clock divide ratio & oscillator frequency */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_CLOCK_DIV_RATIO,
				    config->clock_div_ratio);
	if (ret < 0) { return ret; }

	/* 0xD9, prechargep - Pre-charge period */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_PRECHARGE_PERIOD,
				    config->prechargep);
	if (ret < 0) { return ret; }

	/* 0xDB, vcom - VCOM deselect level */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_VCOM_DESELECT,
				    config->vcom_deselect);
	if (ret < 0) { return ret; }

	/* 0xB0 - Set page address to 0 */
	ret = sh1107_write_cmd_byte(dev, SH1107_SET_PAGE_ADDR);
	if (ret < 0) { return ret; }

	/* 0xDA, 0x12 - COM pins hardware configuration */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_COM_PINS, 0x12);
	if (ret < 0) { return ret; }

	/* 0xAD, 0x8B - DC-DC converter ON */
	ret = sh1107_write_cmd_pair(dev, SH1107_SET_DCDC,
				    config->dcdc_on ? SH1107_DCDC_ON
						   : SH1107_DCDC_OFF);
	if (ret < 0) { return ret; }

	/* 0xA4 - Output follows RAM content */
	ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_RAM);
	if (ret < 0) { return ret; }

	/* 0xA6/0xA7 - Normal/Inverse display */
	ret = sh1107_write_cmd_byte(dev,
		config->inversion_on ? SH1107_DISPLAY_INVERSE
				     : SH1107_DISPLAY_NORMAL);
	if (ret < 0) { return ret; }

	/* Clear display RAM */
	{
		uint8_t buf[64];
		memset(buf, 0x00, sizeof(buf));
		for (uint8_t page = 0; page < 16; page++) {
			sh1107_set_pos(dev, page, 0);
			sh1107_write_data(dev, buf, 64);
		}
	}

	/* 0xAF - Turn display ON */
	ret = sh1107_write_cmd_byte(dev, SH1107_DISPLAY_ON);
	if (ret < 0) { return ret; }
	data->blanking_on = false;

	LOG_INF("SH1107 %ux%u initialized on I2C addr 0x%02x",
		config->width, config->height, config->i2c.addr);

	return 0;
}

/* ── Devicetree instantiation ──────────────────────────────────────────── */

#define SH1107_DEFINE(inst)                                                     \
	static struct sh1107_data sh1107_data_##inst;                           \
									        \
	static const struct sh1107_config sh1107_config_##inst = {              \
		.i2c            = I2C_DT_SPEC_INST_GET(inst),                  \
		.width          = DT_INST_PROP(inst, width),                   \
		.height         = DT_INST_PROP(inst, height),                  \
		.segment_offset = DT_INST_PROP_OR(inst, segment_offset, 0),    \
		.page_offset    = DT_INST_PROP_OR(inst, page_offset, 0),       \
		.display_offset = DT_INST_PROP_OR(inst, display_offset, 0),    \
		.multiplex_ratio = DT_INST_PROP_OR(inst, multiplex_ratio, 127),\
		.prechargep     = DT_INST_PROP_OR(inst, prechargep, 0x22),     \
		.contrast       = DT_INST_PROP_OR(inst, contrast, 0x80),       \
		.clock_div_ratio = DT_INST_PROP_OR(inst, clock_div_ratio, 0x51), \
		.vcom_deselect  = DT_INST_PROP_OR(inst, vcom_deselect, 0x35),  \
		.segment_remap  = DT_INST_PROP(inst, segment_remap),           \
		.com_invdir     = DT_INST_PROP(inst, com_invdir),              \
		.inversion_on   = DT_INST_PROP(inst, inversion_on),            \
		.dcdc_on        = DT_INST_PROP_OR(inst, dcdc_on, true),        \
	};                                                                      \
									        \
	DEVICE_DT_INST_DEFINE(inst, sh1107_init, NULL,                          \
			      &sh1107_data_##inst,                              \
			      &sh1107_config_##inst,                            \
			      POST_KERNEL,                                      \
			      CONFIG_DISPLAY_INIT_PRIORITY,                     \
			      &sh1107_driver_api);

DT_INST_FOREACH_STATUS_OKAY(SH1107_DEFINE)
