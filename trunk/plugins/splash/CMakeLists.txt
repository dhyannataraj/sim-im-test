##################
# splash library #
##################

SET(splash_PLATFORMS COMMON)

SET(splash_SRCS_COMMON splash.cpp)

SET(splash_HDRS_COMMON splash.h)

SIM_ADD_PLUGIN(splash)

# splash.png
INSTALL(FILES splash.png DESTINATION ${SIM_PICT_DIR})
ADD_TO_DIST_TARGET("${CMAKE_CURRENT_SOURCE_DIR}/splash.png")