#################
# icons library #
#################
PROJECT(icons)

SET(icons_LIB_SRCS
        icon.cpp
        iconcfg.cpp
)

SET(icons_UI_FILES
        iconcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${icons_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(icons_LIB_SRCS ${icons_UI_FILES})

ADD_LIBRARY(icons SHARED ${icons_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(icons simlib)

# install target
INSTALL(TARGETS icons RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
