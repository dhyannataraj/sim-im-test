sim_plugin_LTLIBRARIES = floaty.la

floaty_la_SOURCES = \
 floaty.cpp \
 floatywnd.cpp

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

floaty_la_METASOURCES = AUTO
floaty_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
floaty_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/floaty.la

