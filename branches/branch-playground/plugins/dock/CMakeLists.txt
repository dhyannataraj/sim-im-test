################
# dock library #
################
IF(BUILD_DROPPED)
SET(dock_SRCS
        dock.cpp
        dockcfg.cpp
        dockwnd.cpp
)

SET(dock_HDRS
        dock.h
        dockcfg.h
        dockwnd.h
)

SET(dock_UICS
        dockcfgbase.ui
)

SET(dock_LIBS
        _core
)

REMOVE_DEFINITIONS(-DQT3_SUPPORT)
REMOVE_DEFINITIONS(-DQT_3SUPPORT_LIB)
REMOVE_DEFINITIONS(-DQT3_SUPPORT_WARNINGS)
SIM_ADD_PLUGIN(dock)
ENDIF(BUILD_DROPPED)
