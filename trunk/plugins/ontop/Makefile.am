sim_plugin_LTLIBRARIES = ontop.la

ontop_la_SOURCES = \
 ontop.cpp \
 ontopcfg.cpp \
 ontopcfgbase.ui

INCLUDES=-I../../sim $(all_includes)

ontop_la_METASOURCES = AUTO
ontop_la_LIBADD = ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
ontop_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/ontop.la


