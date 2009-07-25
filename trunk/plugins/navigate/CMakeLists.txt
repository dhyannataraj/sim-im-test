####################
# navigate library #
####################
SET(navigate_PLATFORMS COMMON)


SET(navigate_SRCS_COMMON navcfg.cpp
                         navigate.cpp)

SET(navigate_HDRS_COMMON navcfg.h
                         navigate.h)

SET(navigate_UICS_COMMON navcfgbase.ui)

SET(navigate_FLEX_COMMON parseurl.ll)

SIM_ADD_PLUGIN(navigate)
