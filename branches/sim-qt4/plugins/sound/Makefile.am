sim_plugin_LTLIBRARIES = sound.la

sound_la_SOURCES = \
 sound.cpp \
 soundconfig.cpp \
 soundconfigbase.ui \
 sounduser.cpp \
 sounduserbase.ui

INCLUDES=-I../../lib -I../_core $(all_includes)
SUBDIRS=sounds

sound_la_METASOURCES = AUTO
sound_la_LIBADD = ../../lib/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_QT) $(EXTRA_LIBS)
sound_la_LDFLAGS = -module -no-undefined -avoid-version -L../../lib/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/sound.la


