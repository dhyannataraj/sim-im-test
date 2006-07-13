IF(WIN32)
    ###################
    # windock library #
    ###################
    PROJECT(windock)

    SET(windock_LIB_SRCS
            windock.cpp
    )

    # moc, if needed
    KDE3_AUTOMOC(${windock_LIB_SRCS} windock.h)

    ADD_LIBRARY(windock SHARED ${windock_LIB_SRCS})

    # some needed include dirs
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}
                        ${CMAKE_SOURCE_DIR}/plugins/ontop)

    TARGET_LINK_LIBRARIES(windock simlib)

    # install target
    INSTALL(TARGETS windock LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
ELSE(WIN32)
    MESSAGE(STATUS "This plugin can only be used on windows")
ENDIF(WIN32)
