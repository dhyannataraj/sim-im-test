#################
# _core library #
#################
ADD_SUBDIRECTORY(pict)
ADD_SUBDIRECTORY(styles)

SET(_core_PLATFORMS COMMON NON_WIN32)

SET(_core_SRCS_COMMON     arcfg.cpp
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
                          msgview_menu.cpp
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
                          textedit_menu.cpp
                          tmpl.cpp
                          toolbar_container.cpp
                          toolbar_history.cpp
                          toolbar_main.cpp
                          toolbar_msgedit.cpp
                          toolbar_textedit.cpp
                          toolbarcfg.cpp
                          toolsetup.cpp
                          usercfg.cpp
                          userhistorycfg.cpp
                          userlist.cpp
                          userview.cpp
                          userviewcfg.cpp
                          userwnd.cpp )
SET(_core_SRCS_NON_WIN32  libintl.cpp )  # is this really needed? _core (at least without KDE support) is successfully built without it

SET(_core_HDRS_COMMON arcfg.h
                      autoreply.h
                      cfgdlg.h
                      cmenu.h
                      commands.h
                      connectionsettings.h
                      connectwnd.h
                      container.h
                      core.h
                      core_consts.h
                      core_events.h
                      declinedlg.h
                      editmail.h
                      editphone.h
                      filecfg.h
                      filetransfer.h
                      history.h
                      historycfg.h
                      historywnd.h
                      interfacecfg.h
                      kpopup.h
                      logindlg.h
                      maininfo.h
                      mainwin.h
                      manager.h
                      msgauth.h
                      msgcfg.h
                      msgcontacts.h
                      msgedit.h
                      msgfile.h
                      msggen.h
                      msgrecv.h
                      msgsms.h
                      msgurl.h
                      msgview.h
                      newprotocol.h
                      nonim.h
                      pagerdetails.h
                      phonedetails.h
                      plugincfg.h
                      prefcfg.h
                      search.h
                      searchall.h
                      smscfg.h
                      status.h
                      statuswnd.h
                      tmpl.h
                      toolbarcfg.h
                      toolsetup.h
                      usercfg.h
                      userhistorycfg.h
                      userlist.h
                      userview.h
                      userviewcfg.h
                      userwnd.h )

SET(_core_UICS_COMMON arcfgbase.ui
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
                      userviewcfgbase.ui )


IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/GPL-Icons/icondef.xml)
    ADD_JISP_ARCHIVE(GPL-Icons  GPL-Icons.jisp  _core_NON_SRC_TARGETS)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/GPL-Icons/icondef.xml)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jisp/icondef.xml)
    ADD_JISP_ARCHIVE(jisp  sim.jisp  _core_NON_SRC_TARGETS non-free)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/jisp/icondef.xml)

FILE(GLOB _prebuilt emoticons_prebuilt/*.jisp)
INSTALL(FILES ${_prebuilt} DESTINATION ${SIM_ICONS_DIR})
FOREACH(file ${_prebuilt})
  ADD_TO_DIST_TARGET(${file} non-free)
ENDFOREACH(file ${_prebuilt})

SIM_ADD_PLUGIN(_core)
