###############
# osd library #
###############
PROJECT(osd)

SET(osd_LIB_SRCS
        osd.cpp
        osdconfig.cpp
        osdiface.cpp
)

SET(osd_UI_FILES
        osdconfigbase.ui
        osdifacebase.ui
)

# moc, if needed
KDE3_AUTOMOC(${osd_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(osd_LIB_SRCS ${osd_UI_FILES})

ADD_LIBRARY(osd SHARED ${osd_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(osd simlib)

# install target
INSTALL(TARGETS osd LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
