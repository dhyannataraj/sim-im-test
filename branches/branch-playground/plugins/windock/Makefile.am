if ENABLE_WINDOCK
sim_plugin_LTLIBRARIES = windock.la
endif

windock_la_SOURCES = \
 windock.cpp

INCLUDES=-I$(srcdir)/../../sim -I../ontop -I../_core -I$(srcdir)/../_core $(all_includes)

windock_la_METASOURCES = AUTO
windock_la_LIBADD = ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KDEFX) $(EXTRA_LIBS)
windock_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



