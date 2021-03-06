##################
# jabber library #
##################
SET(jabber_SRCS
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

SET(jabber_HDRS
        discoinfo.h
        infoproxy.h
        jabber.h
        jabber_ssl.h
        jabberaboutinfo.h
        jabberadd.h
        jabberbrowser.h
        jabberclient.h
        jabberconfig.h
        jabberhomeinfo.h
        jabberinfo.h
        jabbermessage.h
        jabberpicture.h
        jabbersearch.h
        jabberworkinfo.h
        jidadvsearch.h
        jidsearch.h
)

SET(jabber_UICS
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

SIM_ADD_PLUGIN(jabber)

# install some sounds
INSTALL(FILES sounds/error.wav DESTINATION ${SIM_SOUND_DIR})
