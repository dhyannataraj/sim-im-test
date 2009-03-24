##################
# zodiak library #
##################
SET(zodiak_SRCS
        zodiak.cpp
)

SET(zodiak_HDRS
        zodiak.h
)
SET(zodiak_MESSAGE_SOURCES  ${zodiak_SRCS}  ${zodiak_UICS}  PARENT_SCOPE)

SIM_ADD_PLUGIN(zodiak)
