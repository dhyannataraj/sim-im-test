sim_plugin_LTLIBRARIES = forward.la

forward_la_SOURCES = \
 forward.cpp \
 forwardcfg.cpp \
 forwardcfgbase.ui 

INCLUDES=-I../../sim/api -I../_core $(all_includes)

forward_la_METASOURCES = AUTO
forward_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
forward_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/forward.la


