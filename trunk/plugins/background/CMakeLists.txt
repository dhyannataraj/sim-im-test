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

SET(background_MESSAGE_SOURCES  ${background_SRCS}  ${background_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(background)
