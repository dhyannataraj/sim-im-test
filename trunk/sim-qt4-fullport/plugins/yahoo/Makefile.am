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
 crypt.cpp \
 yahootransformtables.c \
 yahoohttp.cpp \
 md5.c \
 sha.c

INCLUDES=-I../../lib -I../_core $(all_includes)

yahoo_la_METASOURCES = AUTO
yahoo_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

yahoo_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/yahoo.la


