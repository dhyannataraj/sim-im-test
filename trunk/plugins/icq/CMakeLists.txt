###############
# icq library #
###############
PROJECT(icq)

SET(icq_LIB_SRCS
        aboutinfo.cpp
        advsearch.cpp
        aimconfig.cpp
        aiminfo.cpp
        aimsearch.cpp
        aimservices.cpp
        encodingdlg.cpp
        homeinfo.cpp
        icq.cpp
        icqbos.cpp
        icqbuddy.cpp
        icqclient.cpp
        icqconfig.cpp
        icqdirect.cpp
        icqicmb.cpp
        icqinfo.cpp
        icqlists.cpp
        icqlocation.cpp
        icqlogin.cpp
        icqmessage.cpp
        icqpicture.cpp
        icqping.cpp
        icqsearch.cpp
        icqsecure.cpp
        icqservice.cpp
        icqvarious.cpp
        interestsinfo.cpp
        moreinfo.cpp
        pastinfo.cpp
        polling.cpp
        rtfgen.cpp
        securedlg.cpp
        verifydlg.cpp
        warndlg.cpp
        workinfo.cpp
        xml.cpp
)

SET(icq_UI_FILES
        aboutinfobase.ui
        advsearchbase.ui
        aimconfigbase.ui
        aiminfobase.ui
        aimsearchbase.ui
        encodingdlgbase.ui
        homeinfobase.ui
        icqconfigbase.ui
        icqinfobase.ui
        icqpicturebase.ui
        icqsearchbase.ui
        icqsecurebase.ui
        interestsinfobase.ui
        moreinfobase.ui
        pastinfobase.ui
        securedlgbase.ui
        verifydlgbase.ui
        warndlgbase.ui
        workinfobase.ui
)

# moc, if needed
KDE3_AUTOMOC(${icq_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(icq_LIB_SRCS ${icq_UI_FILES})

# and flex :)
ADD_FLEX_FILES(icq_LIB_SRCS rtf.ll)

ADD_LIBRARY(icq SHARED ${icq_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

TARGET_LINK_LIBRARIES(icq simlib)

SET_TARGET_PROPERTIES(icq PROPERTIES PREFIX "")

# install target
INSTALL(TARGETS icq LIBRARY DESTINATION {SIM_PLUGIN_DIR} RUNTIME DESTINATION ${SIM_PLUGIN_DIR})

# install some sounds
INSTALL(FILES sounds/mailpager.wav DESTINATION ${SIM_SOUND_DIR})
INSTALL(FILES sounds/web.wav DESTINATION ${SIM_SOUND_DIR})
