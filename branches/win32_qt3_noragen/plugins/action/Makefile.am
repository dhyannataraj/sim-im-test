sim_plugin_LTLIBRARIES = action.la

action_la_SOURCES = \
 action.cpp \
 actioncfg.cpp \
 actioncfgbase.ui \
 menucfg.cpp \
 menucfgbase.ui \
 additem.cpp \
 additembase.ui

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

action_la_METASOURCES = AUTO
action_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

action_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/action.la


