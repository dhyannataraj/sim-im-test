sim_plugin_LTLIBRARIES = __migrate.la

__migrate_la_SOURCES = \
 migrate.cpp \
 migratedlg.cpp \
 migratedlgbase.ui 

INCLUDES=-I../../sim -I../_core $(all_includes)

__migrate_la_METASOURCES = AUTO
__migrate_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
__migrate_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



