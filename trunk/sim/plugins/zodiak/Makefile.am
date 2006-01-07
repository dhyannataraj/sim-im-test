sim_plugin_LTLIBRARIES = zodiak.la

zodiak_la_SOURCES = \
 zodiak.cpp 

EXTRA_DIST = xpm/*.xpm

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

zodiak_la_METASOURCES = AUTO
zodiak_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
zodiak_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/zodiak.la


