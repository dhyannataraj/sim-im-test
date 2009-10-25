IF(NOT WIN32)
    ADD_SUBDIRECTORY(png)
ENDIF(NOT WIN32)

######################
# target sim library #
######################

SET(sim_LIB_SRCS
		ballonmsg.cpp
		buffer.cpp
		builtinlogger.cpp
		cfg.cpp
		cmddef.cpp
		contacts.cpp
		country.cpp
		datepicker.cpp
		editfile.cpp
		event.cpp
		exec.cpp
		fetch.cpp
		fontedit.cpp
		icons.cpp
		intedit.cpp
		linklabel.cpp
		listview.cpp
		log.cpp
		message.cpp
		misc.cpp
		plugins.cpp
		preview.cpp
		profilemanager.cpp
		propertyhub.cpp
		qchildwidget.cpp
		qcolorbutton.cpp
		qkeybutton.cpp
		sax.cpp
		simfs.cpp
		textshow.cpp
		toolbtn.cpp
		translit.cpp
		unquot.cpp
		xsl.cpp
		qzip/ioapi.c
		qzip/qzip.cpp
		qzip/unzip.c
		contacts/userdata.cpp
		contacts/contactlistprivate.cpp
		contacts/clientuserdata.cpp
		contacts/clientuserdataprivate.cpp
		contacts/clientdataiterator.cpp
		contacts/packettype.cpp
		contacts/contact.cpp
		contacts/group.cpp
		contacts/protocol.cpp
		contacts/client.cpp
		socket/socket.cpp
		socket/serversocketnotify.cpp
		socket/serversocket.cpp
		socket/socketfactory.cpp
		socket/clientsocket.cpp
		socket/tcpclient.cpp
		socket/interfacechecker.cpp
		socket/sslclient.cpp
		socket/simclientsocket.cpp
		socket/simserversocket.cpp
		socket/simsockets.cpp
		socket/ip.cpp
		socket/ipresolver.cpp
	)
SET(sim_LIB_HDRS
		ballonmsg.h
		buffer.h
		builtinlogger.h
		cfg.h
		cmddef.h
		contacts.h
		country.h
		datepicker.h
		editfile.h
		event.h
		exec.h
		fetch.h
		fontedit.h
		icons.h
		intedit.h
		linklabel.h
		listview.h
		log.h
		message.h
		misc.h
		plugins.h
		preview.h
		profilemanager.h
		propertyhub.h
		qchildwidget.h
		qcolorbutton.h
		qkeybutton.h
		sax.h
		simapi.h
		simfs.h
		singleton.h
		textshow.h
		toolbtn.h
		translit.h
		unquot.h
		xsl.h
		qzip/ioapi.h
		qzip/qzip.h
		qzip/unzip.h
		fetchclientprivate.h
		contacts/userdata.h
		contacts/contactlistprivate.h
		contacts/clientuserdata.h
		contacts/clientuserdataprivate.h
		contacts/clientdataiterator.h
		contacts/packettype.h
		contacts/contact.h
		contacts/group.h
		contacts/protocol.h
		contacts/client.h
		socket/socket.h
		socket/socketnotify.h
		socket/serversocketnotify.h
		socket/serversocket.h
		socket/socketfactory.h
		socket/clientsocket.h
		socket/tcpclient.h
		socket/interfacechecker.h
		socket/sslclient.h
		socket/simclientsocket.h
		socket/simserversocket.h
		socket/simsockets.h
		socket/ip.h
		socket/ipresolver.h
	)

# moc the sources
QT4_WRAP_CPP(MOC_SRCS ${sim_LIB_HDRS})
SOURCE_GROUP(MOC-Files FILES ${MOC_SRCS})

# don't forget flex
ADD_FLEX_FILES(sim_LIB_SRCS html.ll)

# our target
ADD_LIBRARY(simlib SHARED ${sim_LIB_SRCS} ${sim_LIB_HDRS} ${MOC_SRCS})

IF(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES VERSION 0.0.0 SOVERSION 0)
ELSE(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR} VERSION 0.0.0 SOVERSION 0)
ENDIF(WIN32 AND NOT MINGW)

#some needed defines
SET_TARGET_PROPERTIES(simlib PROPERTIES DEFINE_SYMBOL SIMAPI_EXPORTS)
ADD_DEFINITIONS(-DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# openssl is optional
IF(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(simlib ${OPENSSL_LIBRARIES} ${OPENSSL_EAY_LIBRARIES})
  IF(NOT WIN32)
    TARGET_LINK_LIBRARIES(simlib crypto)
  ENDIF(NOT WIN32)
ENDIF(OPENSSL_FOUND)

IF(ENABLE_KDE4)
  TARGET_LINK_LIBRARIES(simlib ${KDE4_KDECORE_LIBRARY} ${KDE4_KDEUI_LIBRARY} ${KDE4_KIO_LIBRARY})
ENDIF(ENABLE_KDE4)

# we need wsock32 on win32
if (WIN32)
  TARGET_LINK_LIBRARIES(simlib wsock32 wininet)
endif(WIN32)

# libxml, libxslt and iconv
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBICONV_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_LIBRARIES} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WIN32)
  TARGET_LINK_LIBRARIES(simlib ${LIBICONV_LIBRARIES})
ENDIF(WIN32)

# set correct install path
INSTALL(TARGETS simlib LIBRARY DESTINATION ${LIBDIR} RUNTIME DESTINATION ${LIBDIR} BUNDLE DESTINATION ${LIBDIR})

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

IF(APPLE)
    SET(LIBRARY_OUTPUT_PATH ${LIBDIR})
ENDIF(APPLE)

# unit tests
SET(sim_tests_srcs
    tests/test.cpp
    tests/testbuffer.cpp
    tests/testsocketfactory.cpp
    tests/testclientsocket.cpp
)


SET(sim_tests_hdrs
    tests/testbuffer.h
    tests/testsocketfactory.h
    tests/testclientsocket.h
)

QT4_WRAP_CPP(testmocs ${sim_tests_hdrs})

ADD_EXECUTABLE(test ${sim_tests_srcs} ${testmocs})
TARGET_LINK_LIBRARIES(test simlib ${QT_LIBRARIES})

##################
# sim executable #
##################

# main program
SET(sim_EXE_SRCS
    sim.cpp
)

if(WIN32)
  if(MINGW)
    # resource compilation for mingw
    ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o
                       COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                           -i${CMAKE_CURRENT_SOURCE_DIR}/sim.rc
                                           -o ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
    SET(sim_EXE_SRCS ${sim_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
  else(MINGW)
    SET(sim_EXE_SRCS ${sim_EXE_SRCS} sim.rc)
  endif(MINGW)
endif(WIN32)

# i18n
COMPILE_PO_FILES(${CMAKE_SOURCE_DIR}/po sim_EXE_SRCS)

# sim executable
ADD_EXECUTABLE(${PROGNAME} WIN32 MACOSX_BUNDLE ${sim_EXE_SRCS})

IF(SIM_ENABLE_FPIE)
    SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES COMPILE_FLAGS "${SIM_CXX_FPIE_FLAGS}")
    SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES LINK_FLAGS "${SIM_PIE_LDFLAGS}")
ENDIF(SIM_ENABLE_FPIE)

# only needs simlib
TARGET_LINK_LIBRARIES(${PROGNAME} simlib ${QT_LIBRARIES} ${QT_QTMAIN_LIBRARY})

# set correct install path
INSTALL(TARGETS ${PROGNAME} RUNTIME DESTINATION ${BINDIR} BUNDLE DESTINATION ${BINDIR})

SET_TARGET_PROPERTIES(${PROGNAME} PROPERTIES OUTPUT_NAME ${PROGNAME}${SIM_FLAVOUR})

IF(WIN32)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

IF(NOT WIN32 AND NOT APPLE)
    INSTALL(FILES sim.desktop DESTINATION ${XDG_APPS_DIR})
ENDIF(NOT WIN32 AND NOT APPLE)

# some needed libs for win32
if(WIN32)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/iconv.dll    DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libeay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxml2.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxslt.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/ssleay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/zlib1.dll    DESTINATION ${BINDIR})
endif(WIN32)

