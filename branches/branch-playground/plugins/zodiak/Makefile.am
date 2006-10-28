sim_plugin_LTLIBRARIES = zodiak.la

zodiak_la_SOURCES = \
 zodiak.cpp


INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

zodiak_la_METASOURCES = AUTO
zodiak_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
zodiak_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


dist-hook:
	mkdir $(distdir)/xpm
	cp $(srcdir)/xpm/*.xpm $(distdir)/xpm
