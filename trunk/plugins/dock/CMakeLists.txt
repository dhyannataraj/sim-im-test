################
# dock library #
################

SET(dock_PLATFORMS COMMON)

SET(dock_SRCS_COMMON dock.cpp
                     dockcfg.cpp
                     dockwnd.cpp)

SET(dock_HDRS_COMMON dock.h
                     dockcfg.h
                     dockwnd.h)

SET(dock_UICS_COMMON dockcfgbase.ui)

SIM_ADD_PLUGIN(dock)
