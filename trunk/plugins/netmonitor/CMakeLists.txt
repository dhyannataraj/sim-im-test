######################
# netmonitor library #
######################
SET(netmonitor_PLATFORMS COMMON)

SET(netmonitor_SRCS_COMMON monitor.cpp
                           netmonitor.cpp)

SET(netmonitor_HDRS_COMMON monitor.h
                           netmonitor.h)

SIM_ADD_PLUGIN(netmonitor)
