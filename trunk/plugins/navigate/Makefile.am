sim_plugin_LTLIBRARIES = navigate.la

navigate_la_SOURCES = \
 navigate.cpp \
 navcfg.cpp \
 navcfgbase.ui \
 parseurl.ll

INCLUDES=-I../../sim/api $(all_includes)

navigate_la_METASOURCES = AUTO
navigate_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
navigate_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/navigate.la


