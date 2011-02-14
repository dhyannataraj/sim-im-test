if ENABLE_TRANSPARENT
sim_plugin_LTLIBRARIES = transparent.la
endif

transparent_la_SOURCES = \
 transparent.cpp \
 transparentcfg.cpp \
 transparentcfgbase.ui

EXTRA_transparent_la_SOURCES =\
 transtop.cpp

INCLUDES=-I$(srcdir)/../../sim $(all_includes)

transparent_la_METASOURCES = AUTO
transparent_la_LIBADD = $(TRANSPARENT_OBJ) ../../sim/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KDEFX) $(EXTRA_LIBS)
transparent_la_DEPENDENCIES  = $(TRANSPARENT_OBJ)
transparent_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



