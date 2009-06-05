#######################
# transparent library #
#######################
SET(transparent_SRCS
        transparent.cpp
        transparentcfg.cpp
)

SET(transparent_HDRS
        transparent.h
        transparentcfg.h
)

SET(transparent_UICS
        transparentcfgbase.ui
)

SET(transparent_LIBS
        _core
        floaty
)

SIM_ADD_PLUGIN(transparent)
