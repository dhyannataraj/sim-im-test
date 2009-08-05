SET(update_PLATFORMS COMMON)

SET(update_SRCS_COMMON update.cpp)

SET(update_HDRS_COMMON update.h)

##################
# update library #
##################

# Update plugin is disabled in cmake builds because it really works only in noragen's MSVC builds
# Adding it only for make dist and update-messages purposes
SET(update_PLUGIN_FORBIDDEN 1)
SIM_ADD_PLUGIN(update)

