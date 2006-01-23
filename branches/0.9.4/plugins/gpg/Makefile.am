sim_plugin_LTLIBRARIES = gpg.la

gpg_la_SOURCES = \
 gpg.cpp \
 gpgfindbase.ui \
 gpgcfg.cpp \
 gpgcfgbase.ui \
 gpgadv.cpp \
 gpgadvbase.ui \
 gpggen.cpp \
 gpggenbase.ui \
 gpguser.cpp \
 gpguserbase.ui \
 passphrase.cpp \
 passphrasebase.ui

EXTRA_gpg_la_SOURCES =\
 gpgfind.cpp

INCLUDES=-I../../sim -I../_core $(all_includes)

gpg_la_METASOURCES = AUTO
gpg_la_LIBADD =  $(GPG_OBJ) ../../sim/libsim.la  $(LIBUI) $(LIB_QT) $(LIB_KDECORE) $(EXTRA_LIBS)
gpg_la_DEPENDENCIES =  $(GPG_OBJ)
gpg_la_LDFLAGS = -module -no-undefined -avoid-version -L../../sim/.libs $(QT_LDFLAGS) $(KDE_RPATH) $(all_libraries)

install-data-hook:
	rm -rf $(DESTDIR)$(sim_plugindir)/gpg.la


