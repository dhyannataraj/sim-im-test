sim_plugin_LTLIBRARIES = styles.la

styles_la_SOURCES = \
 stylescfgbase.ui \
 stylescfg.cpp \
 fontconfigbase.ui \
 fontconfig.cpp \
 styles.cpp

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

styles_la_METASOURCES = AUTO
styles_la_LIBADD = ../../sim/api/libsimapi.la ../../sim/ui/libsimui.la $(LIB_QT) $(EXTRA_LIBS) $(XSS_LIBS)
styles_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/styles.la


