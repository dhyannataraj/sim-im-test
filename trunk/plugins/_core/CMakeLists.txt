#################
# _core library #
#################
PROJECT(_core)

ADD_SUBDIRECTORY(pict)
ADD_SUBDIRECTORY(styles)

SET(_core_LIB_SRCS
        arcfg.cpp
        autoreply.cpp
        cfgdlg.cpp
        cmenu.cpp
        commands.cpp
        connectionsettings.cpp
        connectwnd.cpp
        container.cpp
        core.cpp
        declinedlg.cpp
        editmail.cpp
        editphone.cpp
        filecfg.cpp
        filetransfer.cpp
        history.cpp
        historycfg.cpp
        historywnd.cpp
        interfacecfg.cpp
        kpopup.cpp
        logindlg.cpp
        maininfo.cpp
        mainwin.cpp
        manager.cpp
        msgauth.cpp
        msgcfg.cpp
        msgcontacts.cpp
        msgedit.cpp
        msgfile.cpp
        msggen.cpp
        msgrecv.cpp
        msgsms.cpp
        msgurl.cpp
        msgview.cpp
        newprotocol.cpp
        nonim.cpp
        pagerdetails.cpp
        phonedetails.cpp
        plugincfg.cpp
        prefcfg.cpp
        search.cpp
        searchall.cpp
        smscfg.cpp
        status.cpp
        statuswnd.cpp
        tmpl.cpp
        toolbarcfg.cpp
        toolsetup.cpp
        usercfg.cpp
        userhistorycfg.cpp
        userlist.cpp
        userview.cpp
        userviewcfg.cpp
        userwnd.cpp
)

# is this really needed?
IF(NOT WIN32)
SET(_core_LIB_SRCS
        ${_core_LIB_SRCS}
        libintl.cpp
    )
ENDIF(NOT WIN32)

SET(_core_UI_FILES
        arcfgbase.ui
        autoreplybase.ui
        cfgdlgbase.ui
        connectionsettingsbase.ui
        connectwndbase.ui
        declinedlgbase.ui
        editmailbase.ui
        editphonebase.ui
        filecfgbase.ui
        filetransferbase.ui
        fontconfigbase.ui
        historycfgbase.ui
        interfacecfgbase.ui
        logindlgbase.ui
        maininfobase.ui
        managerbase.ui
        msgcfgbase.ui
        newprotocolbase.ui
        nonimbase.ui
        pagerbase.ui
        phonebase.ui
        plugincfgbase.ui
        prefcfgbase.ui
        searchallbase.ui
        searchbase.ui
        smscfgbase.ui
        toolsetupbase.ui
        userhistorycfgbase.ui
        userviewcfgbase.ui
)

# moc, if needed
KDE3_AUTOMOC(${_core_LIB_SRCS})

# don't forget our ui's
QT3_ADD_UI_FILES(_core_LIB_SRCS ${_core_UI_FILES})

# this is all we need for _core library
ADD_LIBRARY(_core SHARED ${_core_LIB_SRCS})

# some needed include dirs
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

# so we don't need an extra CMakeLists.txt in every subdirectory
ADD_JISP_ARCHIVE(additional icq5 icq5.1 icqlite jisp smiles)

# we need simlib (and qt libs, but because simlib depends on them there's no
#                 need to add them here once more)
TARGET_LINK_LIBRARIES(_core simlib)

# install target
INSTALL(TARGETS _core LIBRARY DESTINATION plugins)
