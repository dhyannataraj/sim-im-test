################
# dock library #
################
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

SET(dock_MESSAGE_SOURCES  ${dock_SRCS}  ${dock_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(dock)
