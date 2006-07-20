#################
# ontop library #
#################
PROJECT(ontop)

SET(ontop_LIB_SRCS
        ontop.cpp
        ontopcfg.cpp
)

SET(ontop_LIB_HDRS
        ontop.cpp
        ontopcfg.cpp
)

SET(ontop_UI_FILES
        ontopcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${ontop_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(ontop_LIB_SRCS ${ontop_UI_FILES})

ADD_LIBRARY(ontop SHARED ${ontop_LIB_SRCS} ${ontop_LIB_HDRS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ontop simlib)

SET_TARGET_PROPERTIES(ontop PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS ontop LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
