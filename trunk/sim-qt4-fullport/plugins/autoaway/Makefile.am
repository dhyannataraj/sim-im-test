sim_plugin_LTLIBRARIES = autoaway.la

autoaway_la_SOURCES = \
 autoawaycfgbase.ui \
 autoawaycfg.cpp \
 autoaway.cpp

INCLUDES=-I../../lib -I../_core $(all_includes)

autoaway_la_METASOURCES = AUTO
autoaway_la_LIBADD = ../../lib/libsim.la  $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS) $(XSS_LIBS)
autoaway_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/autoaway.la


