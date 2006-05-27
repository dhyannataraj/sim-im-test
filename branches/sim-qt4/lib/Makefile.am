lib_LTLIBRARIES = libsim.la

libsim_la_SOURCES = \
 aboutdata.cpp \
 ballonmsg.cpp \
 buffer.cpp \
 cfg.cpp \
 cmddef.cpp \
 contacts.cpp \
 country.cpp \
 datepicker.cpp \
 editfile.cpp \
 exec.cpp \
 fetch.cpp \
 fontedit.cpp \
 icons.cpp \
 intedit.cpp \
 linklabel.cpp \
 listview.cpp \
 log.cpp \
 message.cpp \
 plugins.cpp \
 preview.cpp \
 qchildwidget.cpp \
 qcolorbutton.cpp \
 qkeybutton.cpp \
 qzip.cpp \
 sax.cpp \
 simapi.cpp \
 socket.cpp \
 sockfactory.cpp \
 sslclient.cpp \
 textshow.cpp \
 toolbtn.cpp \
 translit.cpp \
 unquot.cpp \
 xsl.cpp 

libsim_la_LIBADD   = $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) $(LIB_XML2) $(LIB_XSLT) $(LIBSSL)

INCLUDES=-Iapi $(all_includes) 

METASOURCES = AUTO

libsim_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(SSL_LDFLAGS) $(all_libraries)
