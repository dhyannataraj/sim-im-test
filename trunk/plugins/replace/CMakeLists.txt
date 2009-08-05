###################
# replace library #
###################

SET(replace_PLATFORMS COMMON)

SET(replace_SRCS_COMMON replace.cpp
                        replacecfg.cpp)

SET(replace_HDRS_COMMON replace.h
                        replacecfg.h)

SET(replace_UICS_COMMON replacecfgbase.ui)

SIM_ADD_PLUGIN(replace)
