sim_plugin_LTLIBRARIES = background.la

background_la_SOURCES = \
 background.cpp \
 bkgndcfgbase.ui \
 bkgndcfg.cpp

INCLUDES=-I../../sim $(all_includes)

background_la_METASOURCES = AUTO
background_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
background_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



