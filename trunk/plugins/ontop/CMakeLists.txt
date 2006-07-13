#################
# ontop library #
#################
PROJECT(ontop)

SET(ontop_LIB_SRCS
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

ADD_LIBRARY(ontop SHARED ${ontop_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(ontop simlib)

# install target
INSTALL(TARGETS ontop RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
