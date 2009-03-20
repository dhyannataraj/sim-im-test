##################
# floaty library #
##################
SET(floaty_SRCS
        floaty.cpp
        floatywnd.cpp
)

SET(floaty_HDRS
        floaty.h
        floatywnd.h
)

SET(floaty_MESSAGE_SOURCES  ${floaty_SRCS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(floaty)
