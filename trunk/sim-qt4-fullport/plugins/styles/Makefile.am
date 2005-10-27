sim_plugin_LTLIBRARIES = styles.la

styles_la_SOURCES = \
 stylescfgbase.ui \
 stylescfg.cpp \
 fontconfigbase.ui \
 fontconfig.cpp \
 styles.cpp

INCLUDES=-I../../lib $(all_includes)

styles_la_METASOURCES = AUTO
styles_la_LIBADD = ../../lib/libsim.la   $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS) 
styles_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/styles.la


