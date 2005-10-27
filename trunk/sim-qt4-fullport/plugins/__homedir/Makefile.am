sim_plugin_LTLIBRARIES = __homedir.la

__homedir_la_SOURCES = \
 homedir.cpp \
 homedircfgbase.ui

INCLUDES=-I../../lib $(all_includes)

__homedir_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
__homedir_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/__homedir.la

