sim_plugin_LTLIBRARIES = dock.la

dock_la_SOURCES = \
 dockcfgbase.ui \
 dockcfg.cpp \
 dockwnd.cpp \
 dock.cpp

INCLUDES=-I../../sim/api -I../_core $(all_includes)

dock_la_METASOURCES = AUTO
dock_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
dock_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/dock.la


