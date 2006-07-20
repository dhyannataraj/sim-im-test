###################
# replace library #
###################
PROJECT(replace)

SET(replace_LIB_SRCS
        replace.cpp
        replacecfg.cpp
)

SET(replace_LIB_HDRS
        replace.h
        replacecfg.h
)

SET(replace_UI_FILES
        replacecfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${replace_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(replace_LIB_SRCS ${replace_UI_FILES})

ADD_LIBRARY(replace SHARED ${replace_LIB_SRCS} ${replace_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(replace simlib)

SET_TARGET_PROPERTIES(replace PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS replace LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
