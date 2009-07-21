##################
# action library #
##################

SET(action_PLATFORMS COMMON)

SET(action_SRCS_COMMON action.cpp
                       actioncfg.cpp
                       additem.cpp
                       menucfg.cpp)

SET(action_HDRS_COMMON action.h
                       actioncfg.h
                       additem.h
                       menucfg.h)

SET(action_UICS_COMMON actioncfgbase.ui
                       additembase.ui
                       menucfgbase.ui)

SIM_ADD_PLUGIN(action)
