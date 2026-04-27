if(CONFIG_ZMK_DISPLAY_STATUS_SCREEN_CUSTOM)
    # Include hello world example as default custom screen
    target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../../examples/hello_world_screen.c)
endif()
