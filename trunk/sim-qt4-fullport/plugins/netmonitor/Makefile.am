sim_plugin_LTLIBRARIES = netmonitor.la

netmonitor_la_SOURCES = \
 netmonitor.cpp \
 monitor.cpp

INCLUDES=-I../../lib $(all_includes)

netmonitor_la_METASOURCES = AUTO
netmonitor_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
netmonitor_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/netmonitor.la


