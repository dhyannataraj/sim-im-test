#################
# icons library #
#################
SET(icons_SRCS
        icon.cpp
        iconcfg.cpp
)

SET(icons_HDRS
        icon.h
        iconcfg.h
)

SET(icons_UICS
        iconcfgbase.ui
)

SET(icons_MESSAGE_SOURCES  ${icons_SRCS}  ${icons_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(icons)
