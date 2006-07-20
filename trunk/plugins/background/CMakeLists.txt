######################
# background library #
######################
PROJECT(background)

SET(background_LIB_SRCS
        background.cpp
        bkgndcfg.cpp
)

SET(background_LIB_HDRS
        background.h
        bkgndcfg.h
)

SET(background_UI_FILES
        bkgndcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${background_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(background_LIB_SRCS ${background_UI_FILES})

ADD_LIBRARY(background SHARED ${background_LIB_SRCS} ${background_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(background simlib)

SET_TARGET_PROPERTIES(background PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS background LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
