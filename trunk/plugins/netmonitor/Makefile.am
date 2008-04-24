sim_plugin_LTLIBRARIES = netmonitor.la

netmonitor_la_SOURCES = \
 netmonitor.cpp \
 monitor.cpp

INCLUDES=$(LTDLINCL) -I$(srcdir)/../../sim -I. -I../../plugins/_core 
$(all_includes)

netmonitor_la_METASOURCES = AUTO
netmonitor_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
netmonitor_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



