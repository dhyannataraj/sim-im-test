#######################
# transparent library #
#######################

SET(transparent_PLATFORMS COMMON NON_WIN32)

SET(transparent_SRCS_COMMON    transparent.cpp
                               transparentcfg.cpp)
SET(transparent_SRCS_NON_WIN32 transtop.cpp)

SET(transparent_HDRS_COMMON    transparent.h
                               transparentcfg.h)
SET(transparent_HDRS_NON_WIN32 transtop.h)

SET(transparent_UICS_COMMON transparentcfgbase.ui)

IF(WIN32 OR ENABLE_KDE3)
    SIM_ADD_PLUGIN(transparent)
ELSE(WIN32 OR ENABLE_KDE3)
    MESSAGE(STATUS "This plugin can only be used on windows or KDE")

    #Adding plugin for update-messages and make dist purposes olny
    SET(transparent_PLUGIN_FORBIDDEN 1)
    SIM_ADD_PLUGIN(transparent)
ENDIF(WIN32 OR ENABLE_KDE3)
