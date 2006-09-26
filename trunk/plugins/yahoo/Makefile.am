if ENABLE_YAHOO
sim_plugin_LTLIBRARIES = yahoo.la
endif

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
 yahoohttp.cpp

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

yahoo_la_METASOURCES = AUTO
yahoo_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

yahoo_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



