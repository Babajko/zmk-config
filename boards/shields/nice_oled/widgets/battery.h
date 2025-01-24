#pragma once

#include <lvgl.h>
#include "util.h"

enum battery_data_source {
	BATTERY_DATA_SOURCE_CENTRAL = 0,
	BATTERY_DATA_SOURCE_PERIPHERAL,
};

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
#if IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING)
	enum battery_data_source source;
	uint8_t p_level;
#endif /* IS_ENABLED(CONFIG_ZMK_SPLIT_BLE_CENTRAL_BATTERY_LEVEL_FETCHING) */
};

void draw_battery_status(lv_obj_t *canvas, const struct status_state *state, const struct util_position *pos);
