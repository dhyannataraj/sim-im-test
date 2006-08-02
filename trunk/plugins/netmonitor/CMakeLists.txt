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

SIM_ADD_PLUGIN(netmonitor)
