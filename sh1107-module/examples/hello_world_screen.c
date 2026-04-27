/*
 * Copyright (c) 2024
 * SPDX-License-Identifier: GPL 
 *
 * Example: Hello World screen for SH1107 OLED
 *
 * Usage in ZMK:
 * 1. Copy this file to your zmk-config project
 * 2. Add to your CMakeLists.txt or shield config
 * 3. Set CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM=y
 */

#include <zephyr/kernel.h>
#include <lvgl.h>

lv_obj_t *zmk_display_status_screen(void) {
    lv_obj_t *screen = lv_obj_create(NULL);
    
    /* Black background */
    lv_obj_set_style_bg_color(screen, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, LV_PART_MAIN);
    
    /* Hello World label */
    lv_obj_t *label = lv_label_create(screen);
    lv_label_set_text(label, "HELLO\nWORLD");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    
    return screen;
}
