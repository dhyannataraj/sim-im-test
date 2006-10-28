###################
# forward library #
###################
SET(forward_SRCS
        forward.cpp
        forwardcfg.cpp
)

SET(forward_HDRS
        forward.h
        forwardcfg.h
)

SET(forward_UICS
        forwardcfgbase.ui
)

SIM_ADD_PLUGIN(forward)
