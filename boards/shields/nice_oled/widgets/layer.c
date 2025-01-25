#include "layer.h"
#include "../assets/custom_fonts.h"
#include <ctype.h> // Para toupper()
#include <zephyr/kernel.h>
#include <lvgl.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// MC: better implementation
static void draw_layer_status_text(lv_obj_t *canvas, const struct status_state *state,
		const struct util_position *pos) {
	lv_draw_label_dsc_t label_dsc;
	init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_LEFT);

	char text[14] = {};
	int result;

	LOG_DBG("draw_layer_status, index: %d, label: %s", state->layer_index, state->layer_label);
	if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
		result = snprintf(text, sizeof(text), "Layer %i", state->layer_index);
		// result = snprintf(text, sizeof(text), LV_SYMBOL_KEYBOARD " %i", state->layer_index);
	} else {
		result = snprintf(text, sizeof(text), "%s", state->layer_label);
		for (int i = 0; text[i] != '\0'; i++) {
			// toupper( ... ): This function, found in the ctype.h library, takes a
			// character as an argument and converts it to its uppercase equivalent.
			// If the character is already uppercase or not a letter, the function
			// returns it unchanged.
			text[i] = toupper(text[i]);
		}
	}

	if (result >= sizeof(text)) {
		LV_LOG_WARN("truncated");
	}
	lv_canvas_draw_text(canvas, pos->x, pos->y, WIDGET_LAYAR_TEXT_WIDTH, &label_dsc, text);
}

static void init_arc_dsc_priv(lv_draw_arc_dsc_t *arc_dsc, lv_color_t color, uint8_t width) {
	lv_draw_arc_dsc_init(arc_dsc);
	arc_dsc->color = color;
	arc_dsc->width = width;
}

static void draw_layer_status_bubbles(lv_obj_t *canvas, const struct status_state *state,
		const struct util_position *pos) {
	lv_draw_rect_dsc_t rect_white_dsc;
	init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
	lv_draw_arc_dsc_t arc_dsc;
	init_arc_dsc_priv(&arc_dsc, LVGL_FOREGROUND, 2);
	lv_draw_arc_dsc_t arc_dsc_filled;
	init_arc_dsc_priv(&arc_dsc_filled, LVGL_FOREGROUND, 9);
	lv_draw_label_dsc_t label_dsc;
	init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);
	lv_draw_label_dsc_t label_dsc_black;
	init_label_dsc(&label_dsc_black, LVGL_BACKGROUND, &lv_font_montserrat_18, LV_TEXT_ALIGN_CENTER);

	// Draw circles
	// int circle_offsets[5][2] = {
	// 		{13, 13},
	// 		{55, 13},
	// 		{34, 34},
	// 		{13, 55},
	// 		{55, 55},
	// };
	const struct util_position circle_offsets[] = {
			{15, 13},
			{49, 13},
			{13, 43},
			{49, 43},
	};

	for (int i = 0; i < count_of(circle_offsets); i++) {
		const bool selected = i == state->layer_index;
		const struct util_position circle_pos = {.x = pos->x + circle_offsets[i].x,
				.y = pos->y + circle_offsets[i].y};

		lv_canvas_draw_arc(canvas, circle_pos.x, circle_pos.y, 13, 0, 360, &arc_dsc);

		if (selected) {
			lv_canvas_draw_arc(canvas, circle_pos.x, circle_pos.y, 9, 0, 359, &arc_dsc_filled);
		}

		char label[2];
		snprintf(label, sizeof(label), "%d", i);
		lv_canvas_draw_text(canvas, circle_pos.x - 8, circle_pos.y - 10, 16,
				(selected ? &label_dsc_black : &label_dsc), label);
	}
}

void draw_layer_status(lv_obj_t *canvas, const struct status_state *state,
		const struct util_position *pos) {
	// draw_layer_status_text(canvas, state, pos);
	draw_layer_status_bubbles(canvas, state, pos);
}