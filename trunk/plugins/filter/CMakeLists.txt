##################
# filter library #
##################
SET(filter_SRCS
        filter.cpp
        filtercfg.cpp
        ignorelist.cpp
)

SET(filter_HDRS
        filter.h
        filtercfg.h
        ignorelist.h
)

SET(filter_UICS
        filtercfgbase.ui
        ignorelistbase.ui
)

SET(filter_MESSAGE_SOURCES  ${filter_SRCS}  ${filter_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(filter)
