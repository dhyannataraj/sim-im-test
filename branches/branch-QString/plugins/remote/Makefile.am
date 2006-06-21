if ENABLE_REMOTE
sim_plugin_LTLIBRARIES = remote.la
bin_PROGRAMS = simctrl
dist_mime_DATA = x-icq.desktop
dist_service_DATA = simctrl.desktop
endif

remote_la_SOURCES = \
 remotecfgbase.ui \
 remotecfg.cpp \
 remote.cpp

simctrl_SOURCES = \
 simctrl.cpp

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

mimedir = $(kde_mimedir)/application
servicedir = $(kde_servicesdir)

remote_la_METASOURCES = AUTO
remote_la_LIBADD = ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
remote_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



