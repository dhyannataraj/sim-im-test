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
 polling.cpp \
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
 advsearch.cpp \
 advsearchbase.ui \
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
 warndlg.cpp \
 warndlgbase.ui \
 encodingdlg.cpp \
 encodingdlgbase.ui \
 rtfgen.cpp \
 rtf.ll \
 verifydlg.cpp \
 verifydlgbase.ui \
 xml.cpp

SUBDIRS = sounds

INCLUDES=-I$(srcdir)/../../sim -I../_core -I$(srcdir)/../_core $(all_includes)

icq_la_METASOURCES = AUTO
icq_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(LIBSSL) $(EXTRA_LIBS)
icq_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)


