##################
# floaty library #
##################
SET(floaty_PLATFORMS COMMON)

SET(floaty_SRCS_COMMON floaty.cpp
                       floatywnd.cpp)

SET(floaty_HDRS_COMMON floaty.h
                       floatywnd.h)

SIM_ADD_PLUGIN(floaty)
