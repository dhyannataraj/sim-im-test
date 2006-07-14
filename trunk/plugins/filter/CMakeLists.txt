##################
# filter library #
##################
PROJECT(filter)

SET(filter_LIB_SRCS
        filter.cpp
        filtercfg.cpp
        ignorelist.cpp
)

SET(filter_UI_FILES
        filtercfgbase.ui
        ignorelistbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${filter_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(filter_LIB_SRCS ${filter_UI_FILES})

ADD_LIBRARY(filter SHARED ${filter_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(filter simlib)

SET_TARGET_PROPERTIES(filter PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS filter LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
