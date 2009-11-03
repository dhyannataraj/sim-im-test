######################
# target simgui library #
######################

SET(simgui_LIB_SRCS
	ballonmsg.cpp
	datepicker.cpp
	editfile.cpp
	fontedit.cpp
	intedit.cpp
	linklabel.cpp
	listview.cpp
	logindlg.cpp
	preview.cpp
	qchildwidget.cpp
	qcolorbutton.cpp
	qkeybutton.cpp
	textshow.cpp
	toolbtn.cpp
	)
SET(simgui_LIB_HDRS
	ballonmsg.h
	datepicker.h
	editfile.h
	fontedit.h
	intedit.h
	linklabel.h
	listview.h
	logindlg.h
	preview.h
	qchildwidget.h
	qcolorbutton.h
	qkeybutton.h
	textshow.h
	toolbtn.h
	)

SET(simgui_ui
	logindlgbase.ui)

# moc the sources
QT4_WRAP_CPP(simgui_MOC_SRCS ${simgui_LIB_HDRS})
QT4_WRAP_UI(simgui_ui_headers ${simgui_ui})
#SOURCE_GROUP(MOC-Files FILES ${MOC_SRCS})

# our target
ADD_LIBRARY(simguilib SHARED ${simgui_LIB_SRCS} ${simgui_MOC_SRCS} ${simgui_ui_headers})

#IF(WIN32 AND NOT MINGW)
    #SET_TARGET_PROPERTIES(simguilib PROPERTIES VERSION 0.0.0 SOVERSION 0)
#ELSE(WIN32 AND NOT MINGW)
    #SET_TARGET_PROPERTIES(simguilib PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR} VERSION 0.0.0 SOVERSION 0)
#ENDIF(WIN32 AND NOT MINGW)

#some needed defines
SET_TARGET_PROPERTIES(simguilib PROPERTIES DEFINE_SYMBOL SIMAPI_EXPORTS)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

#IF(ENABLE_KDE4)
  #TARGET_LINK_LIBRARIES(simguilib ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY})
#ENDIF(ENABLE_KDE4)

TARGET_LINK_LIBRARIES(simguilib ${QT_LIBRARIES})

#IF(WIN32)
  #TARGET_LINK_LIBRARIES(simguilib ${LIBICONV_LIBRARIES})
#ENDIF(WIN32)

# set correct install path
#INSTALL(TARGETS simguilib LIBRARY DESTINATION ${LIBDIR} RUNTIME DESTINATION ${LIBDIR} BUNDLE DESTINATION ${LIBDIR})

#IF(WIN32)
    #SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
#ENDIF(WIN32)

#IF(APPLE)
    #SET(LIBRARY_OUTPUT_PATH ${LIBDIR})
#ENDIF(APPLE)

