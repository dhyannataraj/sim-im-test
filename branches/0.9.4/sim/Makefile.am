bin_PROGRAMS = sim 
lib_LTLIBRARIES = libsim.la

SUBDIRS=png qt3 libltdl

sim_SOURCES = sim.cpp

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
 xsl.cpp \
 unzip.c \
 html.ll

sim_LDADD = libsim.la
libsim_la_LIBADD   = $(LIBLTDL) $(LIB_QT) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(EXTRA_LIBS) $(LIB_XML2) $(LIB_XSLT) $(LIBSSL)

EXTRA_DIST = \
 sim.desktop \
 sim.cpp \
 sim.h

install-data-local:
	$(mkinstalldirs) $(libdir)/menu
	$(INSTALL_DATA) $(srcdir)/sim-kde.menu $(libdir)/menu
	$(mkinstalldirs) $(kde_appsdir)/Internet/
	$(INSTALL_DATA) $(srcdir)/sim.desktop $(kde_appsdir)/Internet/sim.desktop

uninstall-local:
	-rm -f $(kde_appsdir)/Internet/sim.desktop

# set the include path for X, qt and KDE
INCLUDES = $(LTDLINCL) $(all_includes) 

METASOURCES = AUTO

# the library search path. 
sim_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(SSL_LDFLAGS) $(all_libraries)
libsim_la_LDFLAGS = $(QT_LDFLAGS) $(KDE_RPATH) $(SSL_LDFLAGS) $(all_libraries)


