sim_plugin_LTLIBRARIES = __homedir.la

__homedir_la_SOURCES = \
 homedir.cpp 

INCLUDES=-I../../sim/api $(all_includes)

__homedir_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
__homedir_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/__homedir.la

