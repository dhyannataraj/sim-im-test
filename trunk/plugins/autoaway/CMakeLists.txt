IF(NOT WIN32)
    INCLUDE(CheckFunctionExists)
    INCLUDE(CheckLibraryExists)
    #SET(CMAKE_REQUIRED_LIBRARIES "X11 Xext m")
    CHECK_LIBRARY_EXISTS("Xext" "XScreenSaverRegister" "${X11_LIBRARY_DIR}" XSS_IN_XEXT)
    IF(XSS_IN_XEXT)
        SET(XSS_LIB "Xext")
    ELSE(XSS_IN_XEXT)
        CHECK_LIBRARY_EXISTS("Xss" "XScreenSaverRegister" "${X11_LIBRARY_DIR}" XSS_IN_XSS)
        IF(XSS_IN_XSS)
            SET(XSS_LIB "Xss")
        ENDIF(XSS_IN_XSS)
    ENDIF(XSS_IN_XEXT)
    IF(XSS_LIB)
        CHECK_INCLUDE_FILES("X11/extensions/scrnsaver.h" HAVE_X11_EXTENSIONS_SCRNSAVER)
    ELSE(XSS_LIB)
        CHECK_INCLUDE_FILES("Carbon/Carbon.h" HAVE_CARBON_CARBON_H)
    ENDIF(XSS_LIB)
    IF(HAVE_X11_EXTENSIONS_SCRNSAVER OR HAVE_CARBON_CARBON_H)
        SET(COMPILE_PLUGIN TRUE)
    ENDIF(HAVE_X11_EXTENSIONS_SCRNSAVER OR HAVE_CARBON_CARBON_H)
ELSE(NOT WIN32)
    SET(COMPILE_PLUGIN TRUE)
ENDIF(NOT WIN32)

IF(COMPILE_PLUGIN)
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

    TARGET_LINK_LIBRARIES(autoaway simlib ${XSS_LIB})

    SET_TARGET_PROPERTIES(autoaway PROPERTIES PREFIX "")

    # install target
    INSTALL(TARGETS autoaway LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
ELSE(COMPILE_PLUGIN)
    MESSAGE(STATUS "XScreenSaver library not found, autoaway plugin disabled")
ENDIF(COMPILE_PLUGIN)

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
    INSTALL(TARGETS idleui LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
ENDIF(WIN32)
