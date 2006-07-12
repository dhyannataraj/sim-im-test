####################
# autoaway library #
####################
PROJECT(autoaway)

SET(autoaway_LIB_SRCS
        autoaway.cpp
        autoawaycfg.cpp
)

SET(autoaway_UI_FILES
        autoawaycfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${autoaway_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(autoaway_LIB_SRCS ${autoaway_UI_FILES})

ADD_LIBRARY(autoaway SHARED ${autoaway_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(autoaway simlib)

# install target
INSTALL(TARGETS autoaway LIBRARY DESTINATION plugins)


###############################
# idleui library (win32 only) #
###############################
IF(WIN32)
    PROJECT(idleui)

    SET(idleui_LIB_SRCS
            idleui.cpp
    )

    ADD_LIBRARY(idleui SHARED ${idleui_LIB_SRCS})

    # some needed include dirs
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

    TARGET_LINK_LIBRARIES(idleui simlib)

    # install target
    INSTALL(TARGETS idleui LIBRARY DESTINATION plugins)
ENDIF(WIN32)