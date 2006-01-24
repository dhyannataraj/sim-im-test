sim_plugin_LTLIBRARIES = styles.la

styles_la_SOURCES = \
 stylescfgbase.ui \
 stylescfg.cpp \
 fontconfigbase.ui \
 fontconfig.cpp \
 styles.cpp

INCLUDES=-I$(srcdir)/../../sim $(all_includes)

styles_la_METASOURCES = AUTO
styles_la_LIBADD = ../../sim/libsim.la   $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS) 
styles_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



