sim_plugin_LTLIBRARIES = splash.la

splash_la_SOURCES = \
 splash.cpp

INCLUDES=-I$(srcdir)/../../sim $(all_includes)

splash_la_METASOURCES = AUTO
splash_la_LIBADD = ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
splash_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

dist_pict_DATA = splash.png

pictdir = $(kde_datadir)/sim/pict/


