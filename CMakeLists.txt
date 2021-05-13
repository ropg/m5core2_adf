set(COMPONENT_PRIV_REQUIRES audio_sal audio_hal esp_dispatcher esp_peripherals display_service m5core2_axp192 axp192 i2c_manager)

if(CONFIG_AUDIO_BOARD_CUSTOM)
message(STATUS "Current board name is " CONFIG_AUDIO_BOARD_CUSTOM)
list(APPEND COMPONENT_ADD_INCLUDEDIRS ./m5core2_audio ./m5core2_audio_hw)
set(COMPONENT_SRCS
./m5core2_audio/board.c
./m5core2_audio/board_pins_config.c
./m5core2_audio/sdcard_hack/hack_periph_sdcard.c
./m5core2_audio/sdcard_hack/hack_sdcard.c
./m5core2_audio_hw/m5core2_audio_hw.c
)
endif()

register_component()

IF (IDF_VER MATCHES "v4.")
idf_component_get_property(audio_board_lib audio_board COMPONENT_LIB)
set_property(TARGET ${audio_board_lib} APPEND PROPERTY INTERFACE_LINK_LIBRARIES ${COMPONENT_LIB})

ELSEIF (IDF_VER MATCHES "v3.")
set_property(TARGET idf_component_audio_board APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES  $<TARGET_PROPERTY:${COMPONENT_TARGET},INTERFACE_INCLUDE_DIRECTORIES>)

ENDIF (IDF_VER MATCHES "v4.")
