sim_plugin_LTLIBRARIES = _core.la

_core_la_SOURCES = \
 core.cpp \
 exec.cpp \
 kpopup.cpp \
 commands.cpp \
 cmenu.cpp \
 mainwin.cpp \
 userlist.cpp \
 userview.cpp \
 intedit.cpp \
 icons.cpp \
 cfgdlg.cpp \
 cfgdlgbase.ui \
 plugincfg.cpp \
 plugincfgbase.ui \
 prefcfg.cpp \
 prefcfgbase.ui \
 usercfg.cpp \
 toolsetup.cpp \
 toolsetupbase.ui \
 interfacecfg.cpp \
 interfacecfgbase.ui \
 userviewcfg.cpp \
 userviewcfgbase.ui \
 historycfg.cpp \
 historycfgbase.ui \
 maininfo.cpp \
 maininfobase.ui \
 editmail.cpp \
 editmailbase.ui \
 editphone.cpp \
 editphonebase.ui \
 phonedetails.cpp \
 phonebase.ui \
 pagerdetails.cpp \
 pagerbase.ui \
 search.cpp \
 searchbase.ui \
 connectionsettings.cpp \
 connectionsettingsbase.ui \
 connectwnd.cpp \
 connectwndbase.ui \
 logindlg.cpp \
 logindlgbase.ui \
 manager.cpp \
 managerbase.ui \
 newprotocol.cpp \
 newprotocolbase.ui \
 status.cpp \
 statuswnd.cpp \
 qchildwidget.cpp \
 container.cpp \
 userwnd.cpp \
 msgedit.cpp \
 msggen.cpp \
 msgsms.cpp \
 msgfile.cpp \
 msgview.cpp \
 msgauth.cpp \
 msgrecv.cpp \
 history.cpp \
 historywnd.cpp \
 parse.ll \
 libintl.cpp \
 autoreply.cpp \
 autoreplybase.ui \
 msgcfg.cpp \
 msgcfgbase.ui \
 smscfg.cpp \
 smscfgbase.ui \
 arcfg.cpp \
 arcfgbase.ui \
 tmpl.cpp

SUBDIRS=pict

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

_core_la_METASOURCES = AUTO
_core_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
_core_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/_core.la


