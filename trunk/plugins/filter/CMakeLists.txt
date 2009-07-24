##################
# filter library #
##################

SET(filter_PLATFORMS COMMON)

SET(filter_SRCS_COMMON filter.cpp
                       filtercfg.cpp
                       ignorelist.cpp)

SET(filter_HDRS_COMMON filter.h
                       filtercfg.h
                       ignorelist.h)

SET(filter_UICS_COMMON filtercfgbase.ui
                       ignorelistbase.ui)

SIM_ADD_PLUGIN(filter)
