#################
# about library #
#################
PROJECT(about)

SET(about_LIB_SRCS
        about.cpp
        aboutdlg.cpp
)

SET(about_LIB_HDRS
        about.h
        aboutdlg.h
)

SET(about_UI_FILES
        aboutdlgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${about_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(about_LIB_SRCS ${about_UI_FILES})

ADD_LIBRARY(about SHARED ${about_LIB_SRCS} ${about_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(about simlib)

SET_TARGET_PROPERTIES(about PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS about LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
