sim_plugin_LTLIBRARIES = osd.la

osd_la_SOURCES = \
 osd.cpp \
 osdconfig.cpp \
 osdconfigbase.ui \
 osdiface.cpp \
 osdifacebase.ui

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

osd_la_METASOURCES = AUTO
osd_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS)
osd_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/osd.la


