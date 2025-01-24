#include "util.h"
#include <zephyr/kernel.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

#include <zmk/battery.h>
#include <zmk/ble.h>
#include <zmk/display.h>
#include <zmk/endpoints.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/keymap.h>
#include <zmk/usb.h>
#include <zmk/wpm.h>

#include "battery.h"
#include "layer.h"
#include "output.h"
#include "profile.h"
#include "screen.h"
#include "wpm.h"

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

/**
 * Draw canvas
 **/

static void draw_rectangle_with_border(lv_obj_t *canvas, const struct util_position *pos) {
    // static lv_color_t cbuf[CANVAS_HEIGHT * CANVAS_HEIGHT];
    // lv_canvas_set_buffer(canvas, cbuf, CANVAS_HEIGHT, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);

    // lv_canvas_fill_bg(canvas, lv_color_white(), LV_OPA_COVER);

    lv_draw_rect_dsc_t rect_dsc;
    lv_draw_rect_dsc_init(&rect_dsc);
    rect_dsc.border_color = lv_color_black();
    rect_dsc.border_width = 2;

    lv_canvas_draw_rect(canvas, pos->x + 10, pos->y + 10, 40, 100, &rect_dsc);
}

static void draw_canvas(lv_obj_t *widget, lv_color_t cbuf[],
                        const struct status_state *state) {
  lv_obj_t *canvas = lv_obj_get_child(widget, 0);

  const struct util_position zero_pos = {CANVAS_HEIGHT / 2, 0};

  // Draw widgets
  draw_background(canvas);
  draw_output_status(canvas, state, &zero_pos);

  const struct util_position w_battery_pos = {.x = zero_pos.x + 32, .y = zero_pos.y};
  draw_battery_status(canvas, state, &w_battery_pos);

  //   draw_wpm_status(canvas, state, &zero_pos);
  const struct util_position w_profile_pos = {.x = zero_pos.x + CANVAS_WIDTH / 2 - 17,
		  .y = zero_pos.y + 35};
  draw_profile_status(canvas, state, &w_profile_pos);

  const struct util_position w_status_pos = {.x = zero_pos.x, .y = zero_pos.y + 45};
  draw_layer_status(canvas, state, &w_status_pos);

  // Rotate for horizontal display
  rotate_canvas(canvas, cbuf);
}

/**
 * Battery status
 **/

static void set_battery_status(struct zmk_widget_screen *widget,
		struct battery_status_state state) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
	if (state.source == BATTERY_DATA_SOURCE_PERIPHERAL) {
		widget->state.p_level = state.p_level;
	} else if (state.source == BATTERY_DATA_SOURCE_CENTRAL)
#endif /* IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING) */
	{
		widget->state.battery = state.level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
		widget->state.charging = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
	}

	draw_canvas(widget->obj, widget->cbuf, &widget->state);
}

static void battery_status_update_cb(struct battery_status_state state) {
  struct zmk_widget_screen *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_battery_status(widget, state);
  }
}

static struct battery_status_state central_battery_status_get_state(const zmk_event_t *eh) {
	const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);

	return (struct battery_status_state){
			.level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
			.usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
	};
}

#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
static struct battery_status_state peripheral_battery_status_get_state(const zmk_event_t *eh) {
	const struct zmk_peripheral_battery_state_changed *ev =
			as_zmk_peripheral_battery_state_changed(eh);
	LOG_DBG("peripheral_battery_state_changed, source: %d, state_of_charge: %d", ev->source + 1,
			ev->state_of_charge);
	return (struct battery_status_state){
			.source = BATTERY_DATA_SOURCE_PERIPHERAL,
			.p_level = ev->state_of_charge,
	};
}
#endif /* IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING) */

static struct battery_status_state battery_status_get_state(const zmk_event_t *eh) {
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
	if (as_zmk_peripheral_battery_state_changed(eh) != NULL) {
		return peripheral_battery_status_get_state(eh);
	} else
#endif /* IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY) */
	{
		return central_battery_status_get_state(eh);
	}
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_battery_status, struct battery_status_state,
                            battery_status_update_cb, battery_status_get_state);

ZMK_SUBSCRIPTION(widget_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_peripheral_battery_state_changed);
#endif /* IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY) */
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

/**
 * Layer status
 **/

static void set_layer_status(struct zmk_widget_screen *widget,
                             struct layer_status_state state) {
  widget->state.layer_index = state.index;
  widget->state.layer_label = state.label;

  LOG_DBG("Updating layer status, index: %d, label: %s", state.index, state.label);

  draw_canvas(widget->obj, widget->cbuf, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
	struct zmk_widget_screen *widget;
	SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
		set_layer_status(widget, state);
	}
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
	zmk_keymap_layer_index_t index = zmk_keymap_highest_layer_active();
	return (struct layer_status_state){.index = index,
			.label = zmk_keymap_layer_name(zmk_keymap_layer_index_to_id(index))};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state,
                            layer_status_update_cb, layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

/**
 * Output status
 **/

static void set_output_status(struct zmk_widget_screen *widget,
                              const struct output_status_state *state) {
  widget->state.selected_endpoint = state->selected_endpoint;
  widget->state.active_profile_index = state->active_profile_index;
  widget->state.active_profile_connected = state->active_profile_connected;
  widget->state.active_profile_bonded = state->active_profile_bonded;

  LOG_DBG("Updating output status, endpoint transport: %d, profile: %d, connected: %d, bonded: %d",
		  state->selected_endpoint.transport, state->active_profile_index,
		  state->active_profile_connected, state->active_profile_bonded);

  draw_canvas(widget->obj, widget->cbuf, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
  struct zmk_widget_screen *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_output_status(widget, &state);
  }
}

static struct output_status_state
output_status_get_state(const zmk_event_t *_eh) {
  return (struct output_status_state){
      .selected_endpoint = zmk_endpoints_selected(),
      .active_profile_index = zmk_ble_active_profile_index(),
      .active_profile_connected = zmk_ble_active_profile_is_connected(),
      .active_profile_bonded = !zmk_ble_active_profile_is_open(),
  };
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

/**
 * WPM status
 **/

static void set_wpm_status(struct zmk_widget_screen *widget,
                           struct wpm_status_state state) {
  for (int i = 0; i < 9; i++) {
	  widget->state.wpm[i] = widget->state.wpm[i + 1];
  }
  widget->state.wpm[9] = state.wpm;

  draw_canvas(widget->obj, widget->cbuf, &widget->state);
}

static void wpm_status_update_cb(struct wpm_status_state state) {
  struct zmk_widget_screen *widget;
  SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) {
    set_wpm_status(widget, state);
  }
}

struct wpm_status_state wpm_status_get_state(const zmk_event_t *eh) {
  return (struct wpm_status_state){.wpm = zmk_wpm_get_state()};
};

ZMK_DISPLAY_WIDGET_LISTENER(widget_wpm_status, struct wpm_status_state,
                            wpm_status_update_cb, wpm_status_get_state)
ZMK_SUBSCRIPTION(widget_wpm_status, zmk_wpm_state_changed);

/**
 * Initialization
 **/

int zmk_widget_screen_init(struct zmk_widget_screen *widget, lv_obj_t *parent) {
	LOG_DBG("Initializing screen widget");
	widget->obj = lv_obj_create(parent);
	lv_obj_set_size(widget->obj, CANVAS_HEIGHT, CANVAS_WIDTH);

	lv_obj_t *canvas = lv_canvas_create(widget->obj);
	lv_obj_align(canvas, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_canvas_set_buffer(canvas, widget->cbuf, CANVAS_HEIGHT, CANVAS_HEIGHT, LV_IMG_CF_TRUE_COLOR);

	sys_slist_append(&widgets, &widget->node);

	widget_battery_status_init();
	widget_layer_status_init();
	widget_output_status_init();
	widget_wpm_status_init();

	return 0;
}

lv_obj_t *zmk_widget_screen_obj(struct zmk_widget_screen *widget) {
  return widget->obj;
}
