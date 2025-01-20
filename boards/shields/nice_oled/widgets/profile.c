#include "profile.h"
// use custom_fonts.h only for the draw_active_profile_text function
#include "../assets/custom_fonts.h"
#include <stdio.h>
#include <zephyr/kernel.h>

LV_IMG_DECLARE(profiles);

static void draw_inactive_profiles(lv_obj_t *canvas,
                                   const struct status_state *state,
                                   const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);

  const int x = pos->x;
  const int y = pos->y + 137;
  lv_canvas_draw_img(canvas, x, y, &profiles, &img_dsc);
  // lv_canvas_draw_img(canvas, 18, 129, &profiles, &img_dsc);
}

static void draw_active_profile(lv_obj_t *canvas,
                                const struct status_state *state,
                                const struct util_position *pos) {
  lv_draw_rect_dsc_t rect_white_dsc;
  init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);

  int offset = state->active_profile_index * 7;

  const int x = pos->x + offset;
  const int y = pos->y + 137;
  lv_canvas_draw_rect(canvas, x, y, 3, 3, &rect_white_dsc);
  // lv_canvas_draw_rect(canvas, 18 + offset, 129, 3, 3, &rect_white_dsc);
}

// MC: mejor implementación
void draw_active_profile_text(
		lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
	// new label_dsc
	lv_draw_label_dsc_t label_dsc;
	init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono_8, LV_TEXT_ALIGN_LEFT);

	// buffer size should be enough for largest number + null character
	char text[14] = {};
	snprintf(text, sizeof(text), "%d", state->active_profile_index + 1);

	// const int x = pos->x + 25;
	// const int y = pos->y;
	lv_canvas_draw_text(canvas, pos->x, pos->y, 35, &label_dsc, text);
}

void draw_profile_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
	// draw_active_profile_text(canvas, state, pos); //todo: move call of this function to the main screen draw function
	draw_inactive_profiles(canvas, state, pos);
	draw_active_profile(canvas, state, pos);
}
