sim_plugin_LTLIBRARIES = update.la

update_la_SOURCES = \
 update.cpp 

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

update_la_METASOURCES = AUTO
update_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

update_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/update.la


