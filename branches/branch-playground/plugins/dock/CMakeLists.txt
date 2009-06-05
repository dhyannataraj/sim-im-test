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

SET(dock_LIBS
        _core
)

SIM_ADD_PLUGIN(dock)
