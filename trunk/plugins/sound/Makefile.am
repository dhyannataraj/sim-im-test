sim_plugin_LTLIBRARIES = sound.la

sound_la_SOURCES = \
 sound.cpp \
 soundconfig.cpp \
 soundconfigbase.ui \
 sounduser.cpp \
 sounduserbase.ui

INCLUDES=-I../../sim/api -I../../sim/ui -I../_core $(all_includes)
SUBDIRS=sounds

sound_la_METASOURCES = AUTO
sound_la_LIBADD = ../../sim/api/libsimapi.la $(LIBUI) $(LIB_KDECORE) $(LIB_QT) $(EXTRA_LIBS)
sound_la_LDFLAGS = -module -no-undefined -avoid-version $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/sound.la


