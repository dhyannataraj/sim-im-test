sim_plugin_LTLIBRARIES = sms.la

sms_la_SOURCES = \
 sms.cpp \
 smssetup.cpp \
 smssetupbase.ui \
 gsm_ta.cpp \
 serial.cpp 

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)

sms_la_METASOURCES = AUTO
sms_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_KDEUI) $(LIB_KFILE) $(LIB_QT) $(EXTRA_LIBS)

sms_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/sms.la


