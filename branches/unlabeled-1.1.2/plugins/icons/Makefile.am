sim_plugin_LTLIBRARIES = icons.la

icons_la_SOURCES = \
 iconcfgbase.ui \
 iconcfg.cpp \
 icon.cpp \
 icondll.cpp

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

icons_la_METASOURCES = AUTO
icons_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
icons_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/icons.la

nstall-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons/


