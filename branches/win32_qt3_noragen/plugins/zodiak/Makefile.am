sim_plugin_LTLIBRARIES = zodiak.la

zodiak_la_SOURCES = \
 zodiak.cpp 

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

zodiak_la_METASOURCES = AUTO
zodiak_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

zodiak_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/zodiak.la


