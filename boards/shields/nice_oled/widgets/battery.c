#include "battery.h"
#include "../assets/custom_fonts.h"
#include <zephyr/kernel.h>
#include "util.h"

LV_IMG_DECLARE(bolt);

static void draw_level(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
  lv_draw_label_dsc_t label_right_dsc;
  init_label_dsc(&label_right_dsc, LVGL_FOREGROUND, &pixel_operator_mono,
                 LV_TEXT_ALIGN_LEFT);
  // LV_TEXT_ALIGN_RIGHT);

  char text[10] = {};

  sprintf(text, "%i%%", state->battery);
  // sprintf(text, "%i%%", state->battery);
  // x, y, width, dsc, text
  const int x = pos->x;
  const int y = pos->y;
  lv_canvas_draw_text(canvas, x, y, 42, &label_right_dsc, text);
}

static void draw_charging_level(lv_obj_t *canvas,
                                const struct status_state *state, const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);
  lv_draw_label_dsc_t label_right_dsc;
  init_label_dsc(&label_right_dsc, LVGL_FOREGROUND, &pixel_operator_mono,
                 LV_TEXT_ALIGN_LEFT);
  // LV_TEXT_ALIGN_RIGHT);

  char text[10] = {};

  sprintf(text, "%i", state->battery);
  // sprintf(text, "%i%%", state->battery);
  int x = pos->x;
  const int y = pos->y;
  lv_canvas_draw_text(canvas, x, y, 35, &label_right_dsc, text);
  x += 25;
  lv_canvas_draw_img(canvas, x, y, &bolt, &img_dsc);
}

void draw_battery_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
  /*
  lv_draw_label_dsc_t label_left_dsc;
  init_label_dsc(&label_left_dsc, LVGL_FOREGROUND, &pixel_operator_mono,
                 LV_TEXT_ALIGN_LEFT);
  lv_canvas_draw_text(canvas, 0, 19, 25, &label_left_dsc, "BAT");
  */

  if (state->charging) {
    draw_charging_level(canvas, state, pos);
  } else {
    draw_level(canvas, state, pos);
  }
}
