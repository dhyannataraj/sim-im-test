#####################
# shortcuts library #
#####################
SET(shortcuts_SRCS
        mousecfg.cpp
        shortcutcfg.cpp
        shortcuts.cpp
)

SET(shortcuts_HDRS
        mousecfg.h
        shortcutcfg.h
        shortcuts.h
)

SET(shortcuts_UICS
        mousecfgbase.ui
        shortcutcfgbase.ui
)

SET(shortcuts_MESSAGE_SOURCES  ${shortcuts_SRCS}  ${shortcuts_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(shortcuts)
