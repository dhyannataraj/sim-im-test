lib_LTLIBRARIES = libsimapi.la

libsimapi_la_SOURCES = \
 aboutdata.cpp \
 buffer.cpp \
 cfg.cpp \
 cmddef.cpp \
 contacts.cpp \
 country.cpp \
 log.cpp \
 message.cpp \
 plugins.cpp \
 simapi.cpp \
 socket.cpp \
 sockfactory.cpp \
 sslclient.cpp \
 translit.cpp \
 unquote.ll \
 md5_locl.h

libsimapi_la_LIBADD = $(LIBS_SSL) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(EXTRA_LIBS) $(LIBLD)

# set the include path for X, qt and KDE
INCLUDES= $(all_includes) 

METASOURCES = AUTO

# the library search path. 
AM_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

