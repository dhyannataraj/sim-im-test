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

SET(forward_MESSAGE_SOURCES  ${forward_SRCS}  ${forward_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(forward)
