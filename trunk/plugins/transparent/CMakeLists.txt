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

IF(NOT WIN32)
    SET(transparent_SRCS
        ${transparent_SRCS}
        transtop.cpp
    )
ENDIF(NOT WIN32)

SET(transparent_MESSAGE_SOURCES  ${transparent_SRCS}  ${transparent_UICS} )
IF(WIN32)
    SET( transparent_MESSAGE_SOURCES  ${transparent_MESSAGE_SOURCES}  transtop.cpp )
ENDIF(WIN32)
SET( transparent_MESSAGE_SOURCES  ${transparent_MESSAGE_SOURCES} PARENT_SCOPE )

IF(WIN32 OR ENABLE_KDE3)
    SIM_ADD_PLUGIN(transparent)
ELSE(WIN32 OR ENABLE_KDE3)
    MESSAGE(STATUS "This plugin can only be used on windows or KDE")
ENDIF(WIN32 OR ENABLE_KDE3)
