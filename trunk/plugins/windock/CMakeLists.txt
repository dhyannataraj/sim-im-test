IF(WIN32)
    ###################
    # windock library #
    ###################
    SET(windock_SRCS
            windock.cpp
    )

    SET(windock_HDRS
            windock.h
    )

    # some needed include dirs
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/plugins/ontop)

    SIM_ADD_PLUGIN(windock)
ELSE(WIN32)
    MESSAGE(STATUS "Windock plugin can only be used on windows")
ENDIF(WIN32)
