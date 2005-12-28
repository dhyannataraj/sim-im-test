sim_plugin_LTLIBRARIES = forward.la

forward_la_SOURCES = \
 forward.cpp \
 forwardcfg.cpp \
 forwardcfgbase.ui 

INCLUDES=-I../../lib -I../_core $(all_includes)

forward_la_METASOURCES = AUTO
forward_la_LIBADD = ../../lib/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
forward_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/forward.la


