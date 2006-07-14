##################
# action library #
##################
PROJECT(action)

SET(action_LIB_SRCS
        action.cpp
        actioncfg.cpp
        additem.cpp
        menucfg.cpp
)

SET(action_UI_FILES
        actioncfgbase.ui
        additembase.ui
        menucfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${action_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(action_LIB_SRCS ${action_UI_FILES})

ADD_LIBRARY(action SHARED ${action_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                    ${_CORE_SRC_DIR})

TARGET_LINK_LIBRARIES(action simlib)

SET_TARGET_PROPERTIES(action PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS action LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
