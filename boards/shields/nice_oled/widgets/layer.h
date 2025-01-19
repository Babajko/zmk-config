#pragma once

#include <lvgl.h>
#include "util.h"

#define WIDGET_LAYAR_TEXT_WIDTH 68

struct layer_status_state {
    uint8_t index;
    const char *label;
};

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos);