######################
# background library #
######################
SET(background_SRCS
        background.cpp
        bkgndcfg.cpp
)

SET(background_HDRS
        background.h
        bkgndcfg.h
)

SET(background_UICS
        bkgndcfgbase.ui
)

SIM_ADD_PLUGIN(background)
