sim_plugin_LTLIBRARIES = autoaway.la

autoaway_la_SOURCES = autoaway.cpp autoawaycfg.cpp autoawaycfgbase.ui idleui.cpp 

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

autoaway_la_METASOURCES = AUTO
autoaway_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) 
autoaway_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries) -lXss



