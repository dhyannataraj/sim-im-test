lib_LTLIBRARIES = libsimui.la

libsimui_la_SOURCES = \
 textshow.cpp \
 editfile.cpp \
 preview.cpp \
 linklabel.cpp \
 ballonmsg.cpp \
 qcolorbutton.cpp \
 qkeybutton.cpp \
 fontedit.cpp \
 listview.cpp \
 toolbtn.cpp

libsimui_la_LIBADD = $(LIBUIQT) ../api/libsimapi.la $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) $(LIBLD)

# set the include path for X, qt and KDE
INCLUDES= -I../api $(all_includes) 

METASOURCES = AUTO

SUBDIRS=qt3

# the library search path. 
AM_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

