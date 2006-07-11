######################
# target sim library #
######################

set (sim_LIB_SRCS
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
        html.cpp
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

# our target
add_library(simlib SHARED ${sim_LIB_SRCS})

# moc the sources
kde3_automoc(${sim_LIB_SRCS})

#some needed defines
add_definitions(-DSIMAPI_EXPORTS -DNOUNCRYPT)

# additional include dirs - at least ${CMAKE_CURRENT_BINARY_DIR} is needed
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# openssl is optional
if(OPENSSL_FOUND)
  include_directories(${OPENSSL_INCLUDE_DIR})
  target_link_libraries(simlib ${OPENSSL_LIBRARIES} ssleay32)
endif(OPENSSL_FOUND)

# we need wsock32 on win32
if (WIN32)
  target_link_libraries(simlib wsock32)
endif(WIN32)

# libxml, libxslt and zlib
include_directories(${LIBXML2_INCLUDE_DIR} ${LIBXSLT_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
target_link_libraries(simlib ${QT_QTMAIN_LIBRARY} ${QT_QT_LIBRARY} ${LIBXML2_LIBRARIES} ${LIBXSLT_LIBRARIES})

##################
# sim executable #
##################

# main program
add_executable(sim sim.cpp)

# only needs simlib
target_link_libraries(sim simlib)