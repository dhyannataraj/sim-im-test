sim_plugin_LTLIBRARIES = logger.la

logger_la_SOURCES = \
 logger.cpp \
 logconfig.cpp \
 logconfigbase.ui

INCLUDES=-I../../lib $(all_includes)

logger_la_METASOURCES = AUTO
logger_la_LIBADD = ../../lib/libsim.la   $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
logger_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/logger.la

