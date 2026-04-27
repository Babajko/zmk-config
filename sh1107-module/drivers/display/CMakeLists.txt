# SPDX-License-Identifier: GPL 

zephyr_library()

zephyr_library_sources(sh1107.c)

zephyr_library_include_directories(${ZEPHYR_BASE}/drivers/display)
