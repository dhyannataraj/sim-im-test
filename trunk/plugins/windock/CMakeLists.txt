SET(windock_SRCS
    windock.cpp
)

SET(windock_HDRS
    windock.h
)

SET(windock_MESSAGE_SOURCES  ${windock_SRCS}  ${windock_UICS}  PARENT_SCOPE)

IF(WIN32)
    ###################
    # windock library #
    ###################

    # some needed include dirs
    INCLUDE_DIRECTORIES( ${CMAKE_SOURCE_DIR}/plugins/ontop)

    SIM_ADD_PLUGIN(windock)
ELSE(WIN32)
    MESSAGE(STATUS "Windock plugin can only be used on windows")
ENDIF(WIN32)
