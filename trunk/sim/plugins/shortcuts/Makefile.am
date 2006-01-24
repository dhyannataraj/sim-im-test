sim_plugin_LTLIBRARIES = shortcuts.la

shortcuts_la_SOURCES = \
 shortcuts.cpp \
 shortcutcfg.cpp \
 shortcutcfgbase.ui \
 mousecfg.cpp \
 mousecfgbase.ui

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

shortcuts_la_METASOURCES = AUTO
shortcuts_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)
shortcuts_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



