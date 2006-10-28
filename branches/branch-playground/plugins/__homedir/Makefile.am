sim_plugin_LTLIBRARIES = __homedir.la

__homedir_la_SOURCES = \
 homedir.cpp \
 homedircfgbase.ui

EXTRA___homedir_la_SOURCES = \
 homedircfg.cpp

INCLUDES=-I$(srcdir)/../../sim $(all_includes)

__homedir_la_METASOURCES = homedircfg.moc
__homedir_la_LIBADD = $(__HOMEDIR_OBJ) ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
__homedir_la_DEPENDENCIES = $(__HOMEDIR_OBJ)
__homedir_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


