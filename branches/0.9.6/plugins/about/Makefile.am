sim_plugin_LTLIBRARIES = about.la

about_la_SOURCES = \
 about.cpp \
 aboutdlg.cpp \
 aboutdlgbase.ui

# set the include path for X, qt and KDE
INCLUDES=$(LTDLINCL) -I$(srcdir)/../../sim -I. -I../_core $(all_includes)

about_la_METASOURCES = AUTO
about_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS)
about_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



