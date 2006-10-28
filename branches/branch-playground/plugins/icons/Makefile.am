sim_plugin_LTLIBRARIES = icons.la

icons_la_SOURCES = \
 iconcfg.cpp \
 iconcfgbase.ui \
 icon.cpp

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

icons_la_METASOURCES = AUTO
icons_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_KFILE) $(EXTRA_LIBS)
icons_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


nstall-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons/


