sim_plugin_LTLIBRARIES = icons.la

icons_la_SOURCES = \
 iconcfg.cpp \
 iconcfgbase.ui \
 icon.cpp \
 icondll.cpp \
 smilecfgbase.ui \
 smilecfg.cpp \
 smiles.cpp

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

icons_la_METASOURCES = AUTO
icons_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_KFILE) $(EXTRA_LIBS)
icons_la_LDFLAGS = -module -no-undefined -avoid-version $(LIB_XML2) $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/icons.la

nstall-data-local:
	$(mkinstalldirs) $(kde_datadir)/sim/icons/


