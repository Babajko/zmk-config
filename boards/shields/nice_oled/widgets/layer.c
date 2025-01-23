#include "layer.h"
#include "../assets/custom_fonts.h"
#include <ctype.h> // Para toupper()
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// MC: better implementation
void draw_layer_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
	lv_draw_label_dsc_t label_dsc;
	init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono, LV_TEXT_ALIGN_LEFT);

	char text[14] = {};
	int result;

	LOG_DBG("draw_layer_status, index: %d, label: %s", state->layer_index, state->layer_label);
	if (state->layer_label == NULL || strlen(state->layer_label) == 0) {
		// result = snprintf(text, sizeof(text), "Layer %i", state->layer_index);
		result = snprintf(text, sizeof(text), LV_SYMBOL_KEYBOARD " %i", state->layer_index);
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
  lv_canvas_draw_text(canvas, pos->x , pos->y, WIDGET_LAYAR_TEXT_WIDTH, &label_dsc, text);
}
