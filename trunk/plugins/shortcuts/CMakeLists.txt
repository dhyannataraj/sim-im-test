#####################
# shortcuts library #
#####################

SET(shortcuts_PLATFORMS COMMON)

SET(shortcuts_SRCS_COMMON mousecfg.cpp
                          shortcutcfg.cpp
                          shortcuts.cpp)

SET(shortcuts_HDRS_COMMON mousecfg.h
                          shortcutcfg.h
                          shortcuts.h)

SET(shortcuts_UICS_COMMON mousecfgbase.ui
                          shortcutcfgbase.ui)

SIM_ADD_PLUGIN(shortcuts)
