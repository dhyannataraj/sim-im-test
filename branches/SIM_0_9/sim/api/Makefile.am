lib_LTLIBRARIES = libsimapi.la

libsimapi_la_SOURCES = \
 ltdl.c \
 aboutdata.cpp \
 buffer.cpp \
 cfg.cpp \
 cmddef.cpp \
 contacts.cpp \
 country.cpp \
 fetch.cpp \
 log.cpp \
 message.cpp \
 plugins.cpp \
 simapi.cpp \
 socket.cpp \
 sockfactory.cpp \
 sslclient.cpp \
 translit.cpp \
 unquot.cpp \
 html.ll \
 md5_dgst.c

libsimapi_la_LIBADD = $(LIBS_SSL) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS) $(LIBLD)

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) 

METASOURCES = AUTO

# the library search path. 
libsimapi_la_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

