sim_plugin_LTLIBRARIES = filter.la

filter_la_SOURCES = \
 filter.cpp \
 filtercfg.cpp \
 filtercfgbase.ui \
 ignorelist.cpp \
 ignorelistbase.ui

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

filter_la_METASOURCES = AUTO
filter_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
filter_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/filter.la


