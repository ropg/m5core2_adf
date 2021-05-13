#
# "main" pseudo-component makefile.
#
# (Uses default behaviour of compiling all source files in directory, adding 'include' to include path.)

ifdef CONFIG_AUDIO_BOARD_CUSTOM
COMPONENT_ADD_INCLUDEDIRS += ./m5core2_audio_hw
COMPONENT_SRCDIRS += ./m5core2_audio_hw

COMPONENT_ADD_INCLUDEDIRS += ./m5core2_audio
COMPONENT_SRCDIRS += ./m5core2_audio
endif
