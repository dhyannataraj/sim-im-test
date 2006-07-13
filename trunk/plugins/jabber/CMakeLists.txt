##################
# jabber library #
##################
PROJECT(jabber)

SET(jabber_LIB_SRCS
        discoinfo.cpp
        httppoll.cpp
        infoproxy.cpp
        jabber.cpp
        jabber_auth.cpp
        jabber_rosters.cpp
        jabber_ssl.cpp
        jabberaboutinfo.cpp
        jabberadd.cpp
        jabberbrowser.cpp
        jabberclient.cpp
        jabberconfig.cpp
        jabberhomeinfo.cpp
        jabberinfo.cpp
        jabbermessage.cpp
        jabberpicture.cpp
        jabbersearch.cpp
        jabberworkinfo.cpp
        jidadvsearch.cpp
        jidsearch.cpp
)

SET(jabber_UI_FILES
        discoinfobase.ui
        infoproxybase.ui
        jabberaboutinfobase.ui
        jabberaddbase.ui
        jabberconfigbase.ui
        jabberhomeinfobase.ui
        jabberinfobase.ui
        jabberpicturebase.ui
        jabberworkinfobase.ui
        jidadvsearchbase.ui
        jidsearchbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${jabber_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(jabber_LIB_SRCS ${jabber_UI_FILES})

ADD_LIBRARY(jabber SHARED ${jabber_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(jabber simlib)

# install target
INSTALL(TARGETS jabber RUNTIME  DESTINATION ${SIM_PLUGIN_DIR})

# install some sounds
INSTALL(FILES sounds/error.wav DESTINATION ${SIM_SOUND_DIR})
