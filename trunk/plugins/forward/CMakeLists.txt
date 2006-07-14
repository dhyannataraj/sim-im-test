###################
# forward library #
###################
PROJECT(forward)

SET(forward_LIB_SRCS
        forward.cpp
        forwardcfg.cpp
)

SET(forward_UI_FILES
        forwardcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${forward_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(forward_LIB_SRCS ${forward_UI_FILES})

ADD_LIBRARY(forward SHARED ${forward_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(forward simlib)

SET_TARGET_PROPERTIES(forward PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS forward LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
