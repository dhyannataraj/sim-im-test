sim_plugin_LTLIBRARIES = sound.la

sound_la_SOURCES = \
 sound.cpp \
 soundconfig.cpp \
 soundconfigbase.ui \
 sounduser.cpp \
 sounduserbase.ui

INCLUDES=-I../../sim -I../_core $(all_includes)
SUBDIRS=sounds

sound_la_METASOURCES = AUTO
sound_la_LIBADD = ../../sim/libsim.la  $(LIBUI) $(LIB_KDECORE) $(LIB_QT) $(EXTRA_LIBS)
sound_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)



