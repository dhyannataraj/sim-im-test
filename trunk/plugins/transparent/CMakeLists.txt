#######################
# transparent library #
#######################
PROJECT(transparent)

SET(transparent_LIB_SRCS
        transparent.cpp
        transparentcfg.cpp
)

SET(transparent_UI_FILES
        transparentcfgbase.ui
)

IF(NOT WIN32)
    SET(transparent_LIB_SRCS ${transparent_LIB_SRCS}
        transtop.cpp)
ENDIF(NOT WIN32)

# moc, if needed
KDE3_AUTOMOC(${transparent_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(transparent_LIB_SRCS ${transparent_UI_FILES})

ADD_LIBRARY(transparent SHARED ${transparent_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(transparent simlib)

# install target
INSTALL(TARGETS transparent LIBRARY DESTINATION plugins)
