###################
# forward library #
###################

SET(forward_PLATFORMS COMMON)

SET(forward_SRCS_COMMON forward.cpp
                        forwardcfg.cpp)

SET(forward_HDRS_COMMON forward.h
                        forwardcfg.h)

SET(forward_UICS_COMMON forwardcfgbase.ui)

SIM_ADD_PLUGIN(forward)
