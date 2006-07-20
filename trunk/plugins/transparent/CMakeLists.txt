#######################
# transparent library #
#######################
IF(WIN32 OR USE_KDE3)
	PROJECT(transparent)

	SET(transparent_LIB_SRCS
        	transparent.cpp
	        transparentcfg.cpp
	)

	SET(transparent_LIB_HDRS
        	transparent.cpp
	        transparentcfg.cpp
	)

	SET(transparent_UI_FILES
        	transparentcfgbase.ui
	)

	IF(NOT WIN32)
	    SET(transparent_LIB_SRCS ${transparent_LIB_SRCS}
        	transtop.cpp)
	ENDIF(NOT WIN32)

	# moc, if needed
	KDE3_AUTOMOC(${transparent_LIB_SRCS})

	# don't forget our ui's
	QT3_ADD_UI_FILES(transparent_LIB_SRCS ${transparent_UI_FILES})

	ADD_LIBRARY(transparent SHARED ${transparent_LIB_SRCS} ${transparent_LIB_HDRS})

	# some needed include dirs
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

	TARGET_LINK_LIBRARIES(transparent simlib)

	SET_TARGET_PROPERTIES(transparent PROPERTIES PREFIX "")

	# install target
	INSTALL(TARGETS transparent LIBRARY DESTINATION ${SIM_PLUGIN_DIR} RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})
ELSE(WIN32 OR USE_KDE3)
	MESSAGE(STATUS "This plugin can only be used on windows or KDE")
ENDIF(WIN32 OR USE_KDE3)
