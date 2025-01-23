#include "output.h"
#include "../assets/custom_fonts.h"
#include <zephyr/kernel.h>

LV_IMG_DECLARE(bt_no_signal);
LV_IMG_DECLARE(bt_unbonded);
LV_IMG_DECLARE(bt);
LV_IMG_DECLARE(usb);

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
static void draw_usb_connected(lv_obj_t *canvas, const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);

  const int x = pos->x;
  const int y = pos->y + 2;
  lv_canvas_draw_img(canvas, x, y, &usb, &img_dsc);
}

static void draw_ble_unbonded(lv_obj_t *canvas, const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);

  // 36 - 39
  const int x = pos->x;
  const int y = pos->y;
  lv_canvas_draw_img(canvas, x, y, &bt_unbonded, &img_dsc);
}
#endif

static void draw_ble_disconnected(lv_obj_t *canvas, const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);

  const int x = pos->x + 1;
  const int y = pos->y;
  lv_canvas_draw_img(canvas, x, y, &bt_no_signal, &img_dsc);
}

static void draw_ble_connected(lv_obj_t *canvas, const struct util_position *pos) {
  lv_draw_img_dsc_t img_dsc;
  lv_draw_img_dsc_init(&img_dsc);
  const int x = pos->x + 1;
  const int y = pos->y;
  lv_canvas_draw_img(canvas, x, y, &bt, &img_dsc);
}

static void draw_active_profile_text(lv_obj_t *canvas, const struct status_state *state,
		const struct util_position *pos) {
	// new label_dsc
	lv_draw_label_dsc_t label_dsc;
	init_label_dsc(&label_dsc, LVGL_FOREGROUND, &pixel_operator_mono_8, LV_TEXT_ALIGN_LEFT);

	// buffer size should be enough for largest number + null character
	char text[14] = {};
	snprintf(text, sizeof(text), "%d", state->active_profile_index + 1);

	const int x = pos->x + 23;
	lv_canvas_draw_text(canvas, x, pos->y, 35, &label_dsc, text);
}

void draw_output_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos) {
  /*
   * WHITOUT BACKGROUND
  lv_draw_rect_dsc_t rect_white_dsc;
  init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
  lv_canvas_draw_rect(canvas, -3, 32, 24, 15, &rect_white_dsc);
  */

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
  switch (state->selected_endpoint.transport) {
  case ZMK_TRANSPORT_USB:
    draw_usb_connected(canvas, pos);
    break;

  case ZMK_TRANSPORT_BLE:
    if (state->active_profile_bonded) {
      if (state->active_profile_connected) {
        draw_ble_connected(canvas, pos);
      } else {
        draw_ble_disconnected(canvas, pos);
      }
    } else {
      draw_ble_unbonded(canvas, pos);
    }
	draw_active_profile_text(canvas, state, pos);
	break;
  }
#else
  if (state->connected) {
    draw_ble_connected(canvas, pos);
  } else {
    draw_ble_disconnected(canvas, pos);
  }
#endif
}
