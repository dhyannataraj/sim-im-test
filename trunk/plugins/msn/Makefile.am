sim_plugin_LTLIBRARIES = msn.la

msn_la_SOURCES = \
 msn.cpp \
 msnconfig.cpp \
 msnconfigbase.ui \
 msnclient.cpp \
 msnpacket.cpp \
 msninfo.cpp \
 msninfobase.ui \
 msnresult.cpp \
 msnresultbase.ui \
 msnsearch.cpp \
 msnsearchbase.ui 

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

msn_la_METASOURCES = AUTO
msn_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
msn_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/msn.la

