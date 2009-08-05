SET(windock_PLATFORMS WIN32)

SET(windock_SRCS_WIN32 windock.cpp)

SET(windock_HDRS_WIN32 windock.h)

IF(WIN32)
    ###################
    # windock library #
    ###################

    # some needed include dirs
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/plugins/ontop)

    SIM_ADD_PLUGIN(windock)
ELSE(WIN32)
    MESSAGE(STATUS "Windock plugin can only be used on windows")

    # Adding plugin for make dist and update-messages purposes
    SET(windock_PLUGIN_FORBIDDEN 1)
    SIM_ADD_PLUGIN(windock)
ENDIF(WIN32)
