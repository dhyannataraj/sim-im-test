sim_plugin_LTLIBRARIES = remote.la
bin_PROGRAMS = simctrl 

remote_la_SOURCES = \
 remotecfgbase.ui \
 remotecfg.cpp \
 remote.cpp

simctrl_SOURCES = \
 simctrl.cpp

INCLUDES=-I../../sim/api -I../_core $(all_includes)

mime_DATA = x-icq.desktop
mimedir = $(kde_mimedir)/application
service_DATA = simctrl.desktop
servicedir = $(kde_servicesdir)

EXTRA_DIST = $(mime_DATA) $(service_DATA)

remote_la_METASOURCES = AUTO
remote_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS) 
remote_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/remote.la


