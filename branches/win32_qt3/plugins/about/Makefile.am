sim_plugin_LTLIBRARIES = about.la

about_la_SOURCES = \
 about.cpp \
 aboutdlg.cpp \
 aboutdlgbase.ui

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

about_la_METASOURCES = AUTO
about_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
about_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/about.la


