######################
# background library #
######################
SET(background_PLATFORMS COMMON)

SET(background_SRCS_COMMON background.cpp
                           bkgndcfg.cpp)

SET(background_HDRS_COMMON background.h
                           bkgndcfg.h)

SET(background_UICS_COMMON bkgndcfgbase.ui)

SIM_ADD_PLUGIN(background)
