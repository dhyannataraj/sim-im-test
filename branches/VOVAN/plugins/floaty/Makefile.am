sim_plugin_LTLIBRARIES = floaty.la

floaty_la_SOURCES = \
 floaty.cpp \
 floatywnd.cpp

INCLUDES=-I../../sim -I../_core $(all_includes)

floaty_la_METASOURCES = AUTO
floaty_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
floaty_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/floaty.la

