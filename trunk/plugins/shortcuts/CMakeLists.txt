#####################
# shortcuts library #
#####################
PROJECT(shortcuts)

SET(shortcuts_LIB_SRCS
        mousecfg.cpp
        shortcutcfg.cpp
        shortcuts.cpp
)

SET(shortcuts_UI_FILES
        mousecfgbase.ui
        shortcutcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${shortcuts_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(shortcuts_LIB_SRCS ${shortcuts_UI_FILES})

ADD_LIBRARY(shortcuts SHARED ${shortcuts_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(shortcuts simlib)

# install target
INSTALL(TARGETS shortcuts RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
