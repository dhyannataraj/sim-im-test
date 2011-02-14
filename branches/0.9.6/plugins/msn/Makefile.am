if ENABLE_MSN
sim_plugin_LTLIBRARIES = msn.la
endif

msn_la_SOURCES = \
 msn.cpp \
 msnconfig.cpp \
 msnconfigbase.ui \
 msnclient.cpp \
 msnfiletransfer.cpp \
 msnpacket.cpp \
 msninfo.cpp \
 msninfobase.ui \
 msnsearch.cpp \
 msnsearchbase.ui \
 msnhttp.cpp

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

msn_la_METASOURCES = AUTO
msn_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIB_XML2) $(LIBSSL) $(EXTRA_LIBS)
msn_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


