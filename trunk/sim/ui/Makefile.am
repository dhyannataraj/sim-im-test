lib_LTLIBRARIES = libsimui.la

libsimui_la_SOURCES = \
 textshow.cpp \
 editfile.cpp \
 preview.cpp \
 linklabel.cpp \
 ballonmsg.cpp \
 qcolorbutton.cpp \
 qkeybutton.cpp \
 qchildwidget.cpp \
 fontedit.cpp \
 listview.cpp \
 toolbtn.cpp \
 datepicker.cpp \
 intedit.cpp 

libsimui_la_LIBADD = $(LIBUIQT) ../api/libsimapi.la $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) $(LIBDL)

# set the include path for X, qt and KDE
INCLUDES= -I../api $(all_includes) 

METASOURCES = AUTO

SUBDIRS=qt3

# the library search path. 
libsimui_la_LDFLAGS = -no-undefined $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

