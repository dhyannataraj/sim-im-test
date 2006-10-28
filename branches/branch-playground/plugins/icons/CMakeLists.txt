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

SIM_ADD_PLUGIN(icons)
