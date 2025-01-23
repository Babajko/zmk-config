#pragma once

#include <lvgl.h>
#include "util.h"
#include <zmk/keymap.h>

#define WIDGET_LAYAR_TEXT_WIDTH 68

struct layer_status_state {
	zmk_keymap_layer_index_t index;
	const char *label;
};

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos);