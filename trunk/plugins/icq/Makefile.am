sim_plugin_LTLIBRARIES = icq.la

icq_la_SOURCES = \
 icq.cpp \
 icqconfig.cpp \
 icqconfigbase.ui \
 icqclient.cpp \
 icqdirect.cpp \
 icqlogin.cpp \
 icqbos.cpp \
 icqbuddy.cpp \
 icqicmb.cpp \
 icqlists.cpp \
 icqlocation.cpp \
 icqping.cpp \
 icqservice.cpp \
 icqvarious.cpp \
 icqinfo.cpp \
 icqinfobase.ui \
 homeinfo.cpp \
 homeinfobase.ui \
 moreinfo.cpp \
 moreinfobase.ui \
 workinfo.cpp \
 workinfobase.ui \
 aboutinfo.cpp \
 aboutinfobase.ui \
 interestsinfo.cpp \
 interestsinfobase.ui \
 pastinfo.cpp \
 pastinfobase.ui \
 icqsecure.cpp \
 icqsecurebase.ui \
 icqsearch.cpp \
 icqsearchbase.ui \
 searchresult.cpp \
 searchresultbase.ui \
 icqpicture.cpp \
 icqpicturebase.ui \
 icqmessage.cpp \
 securedlg.cpp \
 securedlgbase.ui \
 aimconfig.cpp \
 aimconfigbase.ui \
 aiminfo.cpp \
 aiminfobase.ui \
 aimsearch.cpp \
 aimsearchbase.ui \
 aimservices.cpp \
 rtfgen.cpp \
 rtf.ll \
 xml.cpp

SUBDIRS = sounds

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

icq_la_METASOURCES = AUTO
icq_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_QT) $(EXTRA_LIBS)
icq_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(sim_plugindir)/icq.la

