sim_plugin_LTLIBRARIES = weather.la

weather_la_SOURCES = \
 weather.cpp \
 weathercfg.cpp \
 weathercfgbase.ui \
 wifacecfg.cpp \
 wifacecfgbase.ui

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

weather_la_METASOURCES = AUTO
weather_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

weather_la_LDFLAGS = -module -no-undefined -avoid-version $(LIB_XML2) $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/weather.la


