#include "widgets/screen.h"
#include "widgets/modifiers.h"
// #include "widgets/screen_peripheral.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include "assets/pixel_operator_mono.c"
#include "assets/pixel_operator_mono_12.c"
#include "assets/pixel_operator_mono_8.c"
// auto save conflict
#include "assets/custom_fonts.h"

#if IS_ENABLED(CONFIG_NICE_VIEW_WIDGET_STATUS)
static struct zmk_widget_screen screen_widget;
#endif

static struct zmk_widget_modifiers modifiers_widget;

lv_obj_t *zmk_display_status_screen() {
  lv_obj_t *screen;
  screen = lv_obj_create(NULL);

#if IS_ENABLED(CONFIG_NICE_VIEW_WIDGET_STATUS)
  zmk_widget_screen_init(&screen_widget, screen);
  lv_obj_align(zmk_widget_screen_obj(&screen_widget), LV_ALIGN_TOP_LEFT, 0, 0);
#endif

  zmk_widget_modifiers_init(&modifiers_widget, screen);
  lv_obj_align(zmk_widget_modifiers_obj(&modifiers_widget), LV_ALIGN_BOTTOM_RIGHT, 0, 0);

  return screen;
}
