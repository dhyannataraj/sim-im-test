sim_plugin_LTLIBRARIES = __migrate.la

__migrate_la_SOURCES = \
 migrate.cpp \
 migratedlg.cpp \
 migratedlgbase.ui 

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

__migrate_la_METASOURCES = AUTO
__migrate_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
__migrate_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/__migrate.la


