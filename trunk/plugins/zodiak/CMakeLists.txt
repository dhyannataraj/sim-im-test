##################
# zodiak library #
##################
SET(zodiak_PLATFORMS COMMON)

SET(zodiak_SRCS_COMMON zodiak.cpp)

SET(zodiak_HDRS_COMMON zodiak.h)

SIM_ADD_PLUGIN(zodiak)

FILE(GLOB xpm xpm/*.xpm)
FOREACH(file ${xpm})
  ADD_TO_DIST_TARGET("${file}")
ENDFOREACH(file)
