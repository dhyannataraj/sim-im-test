IF(NOT WIN32)
    ADD_SUBDIRECTORY(png)
ENDIF(NOT WIN32)

######################
# target sim library #
######################

SET(sim_LIB_SRCS
        aboutdata.cpp
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
        qchildwidget.cpp
        qcolorbutton.cpp
        qkeybutton.cpp
        sax.cpp
        socket.cpp
        sockfactory.cpp
        sslclient.cpp
        textshow.cpp
        toolbtn.cpp
        translit.cpp
        unquot.cpp
        xsl.cpp
        qzip/ioapi.c
        qzip/qzip.cpp
        qzip/unzip.c
    )

SET(sim_LIB_HDRS
        aboutdata.h
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
        qchildwidget.h
        qcolorbutton.h
        qkeybutton.h
        sax.h
        simapi.h
        socket.h
        sockfactory.h
        textshow.h
        toolbtn.h
        translit.h
        unquot.h
        xsl.h
        qzip/ioapi.h
        qzip/qzip.h
        qzip/unzip.h
    )

# moc the sources
KDE3_AUTOMOC(${sim_LIB_SRCS})

# don't forget flex
ADD_FLEX_FILES(sim_LIB_SRCS html.ll)

# our target
ADD_LIBRARY(simlib SHARED ${sim_LIB_SRCS} ${sim_LIB_HDRS})

IF(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES VERSION 0.0.0 SOVERSION 0)
ELSE(WIN32 AND NOT MINGW)
    SET_TARGET_PROPERTIES(simlib PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR} VERSION 0.0.0 SOVERSION 0)
ENDIF(WIN32 AND NOT MINGW)

#some needed defines
ADD_DEFINITIONS(-DSIMAPI_EXPORTS -DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# openssl is optional
if(OPENSSL_FOUND)
  LINK_DIRECTORIES(${OPENSSL_LIBRARY_DIR})
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(simlib ${OPENSSL_LIBRARIES} ${OPENSSL_EAY_LIBRARIES})
endif(OPENSSL_FOUND)

IF(USE_KDE3)
  ADD_DEFINITIONS(${KDE3_DEFINITIONS})
  LINK_DIRECTORIES(${KDE3_LIB_DIR})
  TARGET_LINK_LIBRARIES(simlib ${QT_AND_KDECORE_LIBS} kdeui kio)
ENDIF(USE_KDE3)

# we need wsock32 on win32
if (WIN32)
  TARGET_LINK_LIBRARIES(simlib wsock32 wininet)
endif(WIN32)

# libxml, libxslt and iconv
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBICONV_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_QT_LIBRARY} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WIN32)
  TARGET_LINK_LIBRARIES(simlib ${LIBICONV_LIBRARIES})
ENDIF(WIN32)

# set correct install path
INSTALL(TARGETS simlib LIBRARY DESTINATION ${LIBDIR} RUNTIME DESTINATION ${LIBDIR})

IF(WIN32)
    SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

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
ADD_EXECUTABLE(sim WIN32 ${sim_EXE_SRCS})

IF(SIM_ENABLE_FPIE)
    SET_TARGET_PROPERTIES(sim PROPERTIES COMPILE_FLAGS "${SIM_CXX_FPIE_FLAGS}")
    SET_TARGET_PROPERTIES(sim PROPERTIES LINK_FLAGS "${SIM_PIE_LDFLAGS}")
ENDIF(SIM_ENABLE_FPIE)

# only needs simlib
TARGET_LINK_LIBRARIES(sim simlib ${QT_QTMAIN_LIBRARY})

# set correct install path
INSTALL(TARGETS sim RUNTIME DESTINATION ${BINDIR})

SET_TARGET_PROPERTIES(sim PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR})

IF(WIN32)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/${CMAKE_BUILD_TYPE})
ENDIF(WIN32)

IF(NOT WIN32)
    INSTALL(FILES sim.desktop DESTINATION ${XDG_APPS_DIR})
ENDIF(NOT WIN32)

# some needed libs for win32
if(WIN32)
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/iconv.dll    DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libeay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxml2.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/libxslt.dll  DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/ssleay32.dll DESTINATION ${BINDIR})
  INSTALL(FILES ${CMAKE_SOURCE_DIR}/install_win32/zlib1.dll    DESTINATION ${BINDIR})
endif(WIN32)
