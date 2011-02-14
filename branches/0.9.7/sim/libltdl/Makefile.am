if INSTALL_LTDL
noinst_HEADERS = ltdl.h
noinst_LTLIBRARIES = libltdl.la
endif

CLEANFILES = libltdl.la

libltdl_la_SOURCES = ltdl.c
libltdl_la_LIBADD = $(LIBADD_DL)

AM_CFLAGS = -Wno-unused
