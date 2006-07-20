################
# dock library #
################
PROJECT(dock)

SET(dock_LIB_SRCS
        dock.cpp
        dockcfg.cpp
        dockwnd.cpp
)

SET(dock_LIB_HDRS
        dock.h
        dockcfg.h
        dockwnd.h
)

SET(dock_UI_FILES
        dockcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${dock_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(dock_LIB_SRCS ${dock_UI_FILES})

ADD_LIBRARY(dock SHARED ${dock_LIB_SRCS} ${dock_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(dock simlib)

SET_TARGET_PROPERTIES(dock PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS dock LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
