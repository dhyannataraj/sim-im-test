sim_plugin_LTLIBRARIES = proxy.la

proxy_la_SOURCES = \
 proxy.cpp \
 proxycfg.cpp \
 proxycfgbase.ui \
 proxyerror.cpp \
 proxyerrorbase.ui

INCLUDES=-I../../sim/api -I../_core $(all_includes)

proxy_la_METASOURCES = AUTO
proxy_la_LIBADD = ../../sim/api/libsimapi.la $(LIB_QT) $(EXTRA_LIBS)
proxy_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/proxy.la


