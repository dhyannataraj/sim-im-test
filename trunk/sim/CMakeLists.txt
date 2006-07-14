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
        exec.cpp
        fetch.cpp
        fontedit.cpp
        icons.cpp
        intedit.cpp
        linklabel.cpp
        listview.cpp
        log.cpp
        message.cpp
        plugins.cpp
        preview.cpp
        qchildwidget.cpp
        qcolorbutton.cpp
        qkeybutton.cpp
        sax.cpp
        simapi.cpp
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

# moc the sources
KDE3_AUTOMOC(${sim_LIB_SRCS})

# don't forget flex
ADD_FLEX_FILES(sim_LIB_SRCS html.ll)

# our target
ADD_LIBRARY(simlib SHARED ${sim_LIB_SRCS})

SET_TARGET_PROPERTIES(simlib PROPERTIES OUTPUT_NAME sim VERSION 0.0.0 SOVERSION 0)

IF(WIN32)
	SET_TARGET_PROPERTIES(simlib PROPERTIES POSTFIX "lib")
ENDIF(WIN32)

#some needed defines
ADD_DEFINITIONS(-DSIMAPI_EXPORTS -DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# openssl is optional
if(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(simlib ${OPENSSL_LIBRARIES})
  # we need libeay here too...
  # FIXME: move this somehow to FindOpenSSL.cmake...
  IF(WIN32)
    IF(MSVC)
      IF(MSVC_IDE)
        TARGET_LINK_LIBRARIES(simlib optimized libeay32MD debug libeay32MDd)
      ELSE(MSVC_IDE)
        STRING(TOLOWER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE_TOLOWER)
       IF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
          TARGET_LINK_LIBRARIES(simlib libeay32MDd)
        ELSE(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
          TARGET_LINK_LIBRARIES(simlib libeay32MD)
        ENDIF(CMAKE_BUILD_TYPE_TOLOWER MATCHES debug)
      ENDIF(MSVC_IDE)
    ELSE(MSVC)
      TARGET_LINK_LIBRARIES(simlib libeay32)
    ENDIF(MSVC)
  ENDIF(WIN32)
endif(OPENSSL_FOUND)

IF(USE_KDE3)
  ADD_DEFINITIONS(${KDE3_DEFINITIONS})
  LINK_DIRECTORIES(${KDE3_LIB_DIR})
  TARGET_LINK_LIBRARIES(simlib ${QT_AND_KDECORE_LIBS} kdeui kio)
ENDIF(USE_KDE3)

# we need wsock32 on win32
if (WIN32)
  TARGET_LINK_LIBRARIES(simlib wsock32)
endif(WIN32)

# libxml, libxslt and zlib
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_QTMAIN_LIBRARY} ${QT_QT_LIBRARY} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

# set correct install path
INSTALL(TARGETS simlib LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX} RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})

##################
# sim executable #
##################

# main program
ADD_EXECUTABLE(sim sim.cpp sim.rc)

# only needs simlib
TARGET_LINK_LIBRARIES(sim simlib)

# set correct install path
INSTALL(TARGETS sim RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX})
