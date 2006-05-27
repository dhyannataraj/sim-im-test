noinst_LTLIBRARIES = libqzip.la

CLEANFILES = libqzip.la

libqzip_la_SOURCES = \
	ioapi.c \
	ioapi.h \
	unzip.c \
	unzip.h \
	qzip.cpp \
	qzip.h

INCLUDES = $(all_includes)
libqzip_la_LIBADD = $(LIB_QT) $(LIBZ)

AM_CFLAGS = -Wno-unused -Wno-parentheses -DNOUNCRYPT
