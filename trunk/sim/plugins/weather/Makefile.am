sim_plugin_LTLIBRARIES = weather.la

weather_la_SOURCES = \
 weather.cpp \
 weathercfg.cpp \
 weathercfgbase.ui \
 wifacecfg.cpp \
 wifacecfgbase.ui

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

SUBDIRS=jisp

weather_la_METASOURCES = AUTO
weather_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
weather_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/weather.la


