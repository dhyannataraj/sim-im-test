sim_plugin_LTLIBRARIES = splash.la

splash_la_SOURCES = \
 splash.cpp 

INCLUDES=-I../../sim/api $(all_includes)

splash_la_METASOURCES = AUTO
splash_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
splash_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

EXTRA_DIST = splash.png

install-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/pict
	$(INSTALL_DATA) $(EXTRA_DIST) $(kde_datadir)/sim/pict/

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/splash.la

