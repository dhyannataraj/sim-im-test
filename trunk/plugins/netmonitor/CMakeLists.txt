######################
# netmonitor library #
######################
PROJECT(netmonitor)

SET(netmonitor_LIB_SRCS
        monitor.cpp
        netmonitor.cpp
)

# moc, if needed
KDE3_AUTOMOC(${netmonitor_LIB_SRCS})

ADD_LIBRARY(netmonitor SHARED ${netmonitor_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(netmonitor simlib)

# install target
INSTALL(TARGETS netmonitor LIBRARY DESTINATION plugins)
