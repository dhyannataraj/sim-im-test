sim_plugin_LTLIBRARIES = logger.la

logger_la_SOURCES = \
 logger.cpp \
 logconfig.cpp \
 logconfigbase.ui

INCLUDES=-I$(srcdir)/../../sim $(all_includes)

logger_la_METASOURCES = AUTO
logger_la_LIBADD = ../../sim/libsim.la   $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
logger_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


