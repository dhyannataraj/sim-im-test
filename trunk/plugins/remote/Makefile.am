sim_plugin_LTLIBRARIES = remote.la
bin_PROGRAMS = simctrl 

remote_la_SOURCES = \
 remotecfgbase.ui \
 remotecfg.cpp \
 remote.cpp

simctrl_SOURCES = \
 simctrl.c

INCLUDES=-I../../sim/api -I../_core $(all_includes)

remote_la_METASOURCES = AUTO
remote_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS) $(XSS_LIBS)
remote_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/remote.la


