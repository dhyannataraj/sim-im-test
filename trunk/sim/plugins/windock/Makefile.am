sim_plugin_LTLIBRARIES = windock.la

windock_la_SOURCES = \
 windock.cpp

INCLUDES=-I$(srcdir)/../../sim -I../ontop -I../_core -I$(srcdir)/../_core $(all_includes)

windock_la_METASOURCES = AUTO
windock_la_LIBADD = ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KDEFX) $(EXTRA_LIBS)
windock_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/windock.la


