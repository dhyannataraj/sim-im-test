#######################
# transparent library #
#######################
IF(WIN32 OR USE_KDE3)
	SET(transparent_SRCS
        	transparent.cpp
	        transparentcfg.cpp
	)

	SET(transparent_HDRS
        	transparent.h
	        transparentcfg.h
	)

	SET(transparent_UICS
        	transparentcfgbase.ui
	)

	IF(NOT WIN32)
	    SET(transparent_SRCS ${transparent_SRCS}
        	transtop.cpp)
	ENDIF(NOT WIN32)

    SIM_ADD_PLUGIN(transparent)
ELSE(WIN32 OR USE_KDE3)
	MESSAGE(STATUS "This plugin can only be used on windows or KDE")
ENDIF(WIN32 OR USE_KDE3)
