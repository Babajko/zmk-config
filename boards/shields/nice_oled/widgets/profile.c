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

  lv_canvas_draw_img(canvas, pos->x, pos->y, &profiles, &img_dsc);
}

static void draw_active_profile(lv_obj_t *canvas,
                                const struct status_state *state,
                                const struct util_position *pos) {
  lv_draw_rect_dsc_t rect_white_dsc;
  init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);

  int offset = state->active_profile_index * 7;

  const int x = pos->x + offset;
  const int y = pos->y;
  lv_canvas_draw_rect(canvas, x, y, 3, 3, &rect_white_dsc);
}

void draw_profile_status(
		lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
	draw_inactive_profiles(canvas, state, pos);
	draw_active_profile(canvas, state, pos);
}
