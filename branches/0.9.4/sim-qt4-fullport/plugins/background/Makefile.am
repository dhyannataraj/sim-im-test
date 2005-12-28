sim_plugin_LTLIBRARIES = background.la

background_la_SOURCES = \
 background.cpp \
 bkgndcfgbase.ui \
 bkgndcfg.cpp

INCLUDES=-I../../lib $(all_includes)

background_la_METASOURCES = AUTO
background_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
background_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/background.la


