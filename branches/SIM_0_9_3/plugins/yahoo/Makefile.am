sim_plugin_LTLIBRARIES = yahoo.la

yahoo_la_SOURCES = \
 yahoo.cpp \
 yahooclient.cpp \
 yahooauth.cpp \
 yahoocfg.cpp \
 yahoocfgbase.ui \
 yahooinfo.cpp \
 yahooinfobase.ui \
 yahoosearch.cpp \
 yahoosearchbase.ui \
 yahooresult.cpp \
 yahooresultbase.ui \
 crypt.c \
 yahootransformtables.c \
 yahoohttp.cpp

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

yahoo_la_METASOURCES = AUTO
yahoo_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

yahoo_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/yahoo.la


