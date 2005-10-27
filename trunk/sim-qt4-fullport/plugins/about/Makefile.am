sim_plugin_LTLIBRARIES = about.la

about_la_SOURCES = \
 about.cpp \
 aboutdlg.cpp \
 aboutdlgbase.ui

INCLUDES=-I../../lib $(all_includes)

about_la_METASOURCES = AUTO
about_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS)
about_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/about.la


