sim_plugin_LTLIBRARIES = jabber.la

jabber_la_SOURCES = \
 jabber.cpp \
 jabberconfig.cpp \
 jabberconfigbase.ui \
 jabberclient.cpp \
 jabber_auth.cpp \
 jabber_rosters.cpp \
 jabber_ssl.cpp \
 jabberinfo.cpp \
 jabberinfobase.ui \
 jabberhomeinfo.cpp \
 jabberhomeinfobase.ui \
 jabberworkinfo.cpp \
 jabberworkinfobase.ui \
 jabberaboutinfo.cpp \
 jabberaboutinfobase.ui \
 jabberadd.cpp \
 jabberaddbase.ui \
 addresult.cpp \
 addresultbase.ui \
 jabbersearch.cpp \
 jabbermessage.cpp \
 jabberpicture.cpp \
 jabberpicturebase.ui \
 services.cpp \
 servicesbase.ui \
 jabberbrowser.cpp \
 discoinfo.cpp \
 discoinfobase.ui

SUBDIRS = sounds

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

jabber_la_METASOURCES = AUTO
jabber_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
jabber_la_LDFLAGS = -module -no-undefined -avoid-version $(LIB_EXPAT) $(LIBS_SSL) $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/jabber.la

