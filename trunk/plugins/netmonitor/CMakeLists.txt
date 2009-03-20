######################
# netmonitor library #
######################
PROJECT(netmonitor)

SET(netmonitor_SRCS
        monitor.cpp
        netmonitor.cpp
)

SET(netmonitor_HDRS
        monitor.h
        netmonitor.h
)

SET(netmonitor_MESSAGE_SOURCES  ${netmonitor_SRCS} PARENT_SCOPE)

SIM_ADD_PLUGIN(netmonitor)
