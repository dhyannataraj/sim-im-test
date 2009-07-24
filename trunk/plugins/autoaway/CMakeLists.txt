####################
# autoaway library #
####################

SET(autoaway_PLATFORMS COMMON WIN32)

SET(autoaway_SRCS_COMMON autoaway.cpp
                         autoawaycfg.cpp)
SET(autoaway_SRCS_WIN32  idleui.cpp)


SET(autoaway_HDRS_COMMON autoaway.h
                         autoawaycfg.h)
SET(autoaway_HDRS_WIN32  idleui.h)


SET(autoaway_UICS_COMMON autoawaycfgbase.ui)

IF(NOT WIN32)
    INCLUDE(CheckFunctionExists)
    INCLUDE(FindX11)
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
    ELSE(HAVE_X11_EXTENSIONS_SCRNSAVER OR HAVE_CARBON_CARBON_H)
        IF( X11_Xscreensaver_FOUND )
            SET(COMPILE_PLUGIN TRUE)
        ENDIF( X11_Xscreensaver_FOUND )
    ENDIF(HAVE_X11_EXTENSIONS_SCRNSAVER OR HAVE_CARBON_CARBON_H)
ELSE(NOT WIN32)
    SET(COMPILE_PLUGIN TRUE)
ENDIF(NOT WIN32)

IF(COMPILE_PLUGIN)
    LINK_DIRECTORIES(${X11_LIBRARY_DIR})
    INCLUDE_DIRECTORIES(${X11_INCLUDE_DIR})
    SIM_ADD_PLUGIN(autoaway)
    TARGET_LINK_LIBRARIES(autoaway ${XSS_LIB})
ELSE(COMPILE_PLUGIN)
    MESSAGE(STATUS "XScreenSaver library not found, autoaway plugin disabled")
    SET(autoaway_PLUGIN_FORBIDDEN 1)
    SIM_ADD_PLUGIN(autoaway) # for update-messages and make dist purposes
ENDIF(COMPILE_PLUGIN)
