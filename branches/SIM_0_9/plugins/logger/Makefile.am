sim_plugin_LTLIBRARIES = logger.la

logger_la_SOURCES = \
 logger.cpp \
 logconfig.cpp \
 logconfigbase.ui

INCLUDES=-I../../sim/api -I../../sim/ui $(all_includes)

logger_la_METASOURCES = AUTO
logger_la_LIBADD = ../../sim/api/libsimapi.la ../../sim/ui/libsimui.la $(LIB_QT) $(EXTRA_LIBS)
logger_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/logger.la

