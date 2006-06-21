if ENABLE_LIVEJOURNAL
sim_plugin_LTLIBRARIES = livejournal.la
endif

livejournal_la_SOURCES = \
 livejournal.cpp \
 livejournalcfg.cpp \
 livejournalcfgbase.ui \
 msgjournal.cpp \
 msgjournalbase.ui \
 journalsearch.cpp \
 journalsearchbase.ui

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

livejournal_la_METASOURCES = AUTO
livejournal_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(LIBSSL) $(EXTRA_LIBS)
livejournal_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



