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

#some needed defines
ADD_DEFINITIONS(-DSIMAPI_EXPORTS -DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# openssl is optional
if(OPENSSL_FOUND)
  INCLUDE_DIRECTORIES(${OPENSSL_INCLUDE_DIR})
  TARGET_LINK_LIBRARIES(simlib ${OPENSSL_LIBRARIES} ssleay32)
endif(OPENSSL_FOUND)

# we need wsock32 on win32
if (WIN32)
  TARGET_LINK_LIBRARIES(simlib wsock32)
endif(WIN32)

# libxml, libxslt and zlib
INCLUDE_DIRECTORIES(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
TARGET_LINK_LIBRARIES(simlib ${QT_QTMAIN_LIBRARY} ${QT_QT_LIBRARY} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})


##################
# sim executable #
##################

# main program
ADD_EXECUTABLE(sim sim.cpp)

# only needs simlib
TARGET_LINK_LIBRARIES(sim simlib)
