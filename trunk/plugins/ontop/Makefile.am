sim_plugin_LTLIBRARIES = ontop.la

ontop_la_SOURCES = \
 ontop.cpp \
 ontopcfg.cpp \
 ontopcfgbase.ui

INCLUDES=-I../../sim/api $(all_includes)

ontop_la_METASOURCES = AUTO
ontop_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
ontop_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/ontop.la


