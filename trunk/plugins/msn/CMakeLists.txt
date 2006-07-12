###############
# msn library #
###############
PROJECT(msn)

SET(msn_LIB_SRCS
msn.cpp
        msnclient.cpp
        msnconfig.cpp
        msnhttp.cpp
        msninfo.cpp
        msnpacket.cpp
        msnsearch.cpp
)

SET(msn_UI_FILES
        msnconfigbase.ui
        msninfobase.ui
        msnsearchbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${msn_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(msn_LIB_SRCS ${msn_UI_FILES})

ADD_LIBRARY(msn SHARED ${msn_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(msn simlib)

# install target
INSTALL(TARGETS msn LIBRARY DESTINATION plugins)
