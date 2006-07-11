################
# _core library
################
project(_core)
add_subdirectory(additional)

set (_core_LIB_SRCS
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
if(NOT WIN32)
set (_core_LIB_SRCS
        ${_core_LIB_SRCS}
        libintl.cpp
    )
endif(NOT WIN32)

set( _core_UI_FILES
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


kde3_automoc(${_core_LIB_SRCS})
qt3_add_ui_files(_core_LIB_SRCS ${_core_UI_FILES})

add_library(_core SHARED ${_core_LIB_SRCS})

# FIXME: ask Alex why we need this (otherwise .moc aren't found)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${SIMAPI_SRC_DIR})
message (STATUS "cur: " ${SIMAPI_SRC_DIR})

target_link_libraries(_core simlib)
