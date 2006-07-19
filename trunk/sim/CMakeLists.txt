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
  TARGET_LINK_LIBRARIES(simlib wsock32)
endif(WIN32)

# libxml, libxslt and iconv
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${LIBICONV_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_QT_LIBRARY} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES} ${ZLIB_LIBRARIES})

IF(WIN32)
  TARGET_LINK_LIBRARIES(simlib ${LIBICONV_LIBRARIES})
ENDIF(WIN32)

# set correct install path
INSTALL(TARGETS simlib LIBRARY DESTINATION ${LIBDIR} RUNTIME DESTINATION ${LIBDIR})

##################
# sim executable #
##################

# main program
SET(sim_EXE_SRCS
    sim.cpp
)
if(MINGW)
  # resource compilation for mingw
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o
                     COMMAND windres.exe -I${CMAKE_CURRENT_SOURCE_DIR}
                                         -i${CMAKE_CURRENT_SOURCE_DIR}/sim.rc
                                         -o ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
  SET(sim_EXE_SRCS ${sim_EXE_SRCS} ${CMAKE_CURRENT_BINARY_DIR}/sim_rc.o)
else(MINGW)
  SET(sim_EXE_SRCS ${sim_EXE_SRCS})
endif(MINGW)

# i18n
COMPILE_PO_FILES(${CMAKE_SOURCE_DIR}/po sim_EXE_SRCS)

# sim executable
ADD_EXECUTABLE(sim WIN32 ${sim_EXE_SRCS})

# only needs simlib
TARGET_LINK_LIBRARIES(sim simlib ${QT_QTMAIN_LIBRARY})

# set correct install path
INSTALL(TARGETS sim RUNTIME DESTINATION ${BINDIR})

SET_TARGET_PROPERTIES(sim PROPERTIES OUTPUT_NAME sim${SIM_FLAVOUR})
